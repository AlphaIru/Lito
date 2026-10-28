// The magic starts here.

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <llvm/Support/CommandLine.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <llvm/ADT/SmallString.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/raw_ostream.h>

#include "codegen.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"

void compileExecutable(const CodeGenerator &generator,
                       const std::string &outputPath);

int main(int argc, char *argv[])
{
    // Parse command-line arguments.
    bool showLLVM = false;

    if (argc < 2)
    {
        std::cerr << "Usage: lito <file.lit> [-o <output> | --show-llvm]\n";
        return 1;
    }

    // Default output name: source filename without extension.
    std::filesystem::path sourcePath(argv[1]);
    std::string outputPath = sourcePath.stem().string();

    if (argc == 3 && std::string(argv[2]) == "--show-llvm")
    {
        showLLVM = true;
    }
    else if (argc == 4 && std::string(argv[2]) == "-o" && argv[3][0] != '\0')
    {
        outputPath = argv[3];
    }
    else if (argc != 2)
    {
        std::cerr << "Usage: lito <file.lit> [-o <output> | --emit-llvm]\n";
        return 1;
    }

    if (outputPath.empty())
    {
        std::cerr << "Invalid output filename\n";
        return 1;
    }

    // Open source file.
    std::ifstream litoFile(argv[1], std::ios::binary);

    if (!litoFile)
    {
        std::cerr << "Cannot open file: " << argv[1] << '\n';
        return 1;
    }

    // Read source code.
    std::string source{std::istreambuf_iterator<char>(litoFile),
                       std::istreambuf_iterator<char>()};

    if (litoFile.bad())
    {
        std::cerr << "Failed to read file: " << argv[1] << '\n';
        return 1;
    }

    // Initialize lexer.
    Lexer lexer(source);

    std::size_t lastPercent = 0;
    std::size_t tokenCount = 0;

    std::cerr << "Lexing: 0%" << std::flush;

    // Process tokens.
    while (true)
    {
        Token token = lexer.nextToken();

        if (token.type == TokenType::Invalid)
        {
            std::cerr << '\n'
                      << argv[1] << ":" << token.line << ":" << token.column
                      << ": Invalid token: " << token.text << '\n';

            return 1;
        }

        // Update progress only when percentage changes.
        std::size_t percent = lexer.progressPercent();

        if (percent != lastPercent)
        {
            std::cerr << "\rLexing: " << percent << "%" << std::flush;

            lastPercent = percent;
        }

        if (token.type == TokenType::EndOfFile)
        {
            break;
        }

        tokenCount++;
    }

    std::cerr << '\n';
    std::cerr << "Tokens: " << tokenCount << '\n';

    // Initialize parser.
    Lexer parserLexer(std::move(source));
    Parser parser(parserLexer);

    try
    {
        // Parse the entire program.
        std::vector<std::unique_ptr<Stmt>> program = parser.parseProgram();

        // Currently, compile one expression statement.
        if (program.size() != 1)
        {
            throw std::runtime_error(
                "Expected exactly one expression statement");
        }

        const ExprStmt *statement =
            dynamic_cast<const ExprStmt *>(program[0].get());

        if (statement == nullptr)
        {
            throw std::runtime_error("Expected an expression statement");
        }

        std::cerr << "Parsing: OK\n";

        // Check for a function call.
        const CallExpr *call =
            dynamic_cast<const CallExpr *>(statement->expression.get());

        if (call == nullptr)
        {
            throw std::runtime_error("Expected a function call");
        }

        // Generate LLVM IR.
        CodeGenerator generator;

        generator.generatePrint(*call);

        // Compile or display LLVM IR.
        if (showLLVM)
        {
            generator.printIR(llvm::outs());
        }

        compileExecutable(generator, outputPath);

        std::cerr << "Compiled: " << outputPath << '\n';
    }
    catch (const std::exception &error)
    {
        std::cerr << "Lito error: " << error.what() << '\n';

        return 1;
    }

    return 0;
}

// Compile LLVM IR into a native executable.
void compileExecutable(const CodeGenerator &generator,
                       const std::string &outputPath)
{
    // Find Clang.
    llvm::ErrorOr<std::string> clangPath =
        llvm::sys::findProgramByName("clang");

    if (!clangPath)
    {
        throw std::runtime_error("Clang was not found");
    }

    // Create a temp LLVM IR file.
    int fileDescriptor = -1;
    llvm::SmallString<128> temporaryPath;

    std::error_code error = llvm::sys::fs::createTemporaryFile(
        "lito", "ll", fileDescriptor, temporaryPath);

    if (error)
    {
        throw std::runtime_error("Failed to create temporary file: " +
                                 error.message());
    }

    // Write LLVM IR.
    {
        llvm::raw_fd_ostream output(fileDescriptor, true);

        generator.printIR(output);
        output.close();

        if (output.has_error())
        {
            llvm::sys::fs::remove(temporaryPath);

            throw std::runtime_error("Failed to write LLVM IR");
        }
    }

    // Execute Clang without a shell.
    llvm::StringRef arguments[] = {
        "clang", "-x", "ir", temporaryPath.str(), "-o", outputPath};

    int result = llvm::sys::ExecuteAndWait(*clangPath, arguments);

    // Remove the temporary IR file.
    llvm::sys::fs::remove(temporaryPath);

    if (result != 0)
    {
        throw std::runtime_error("Clang compilation failed");
    }
}
