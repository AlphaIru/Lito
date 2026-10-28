// LLVM integer code generation tests.

#include <iostream>
#include <memory>
#include <string>

#include "codegen.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <llvm/Support/raw_ostream.h>

// Check the LLVM IR generated from a Lito expression.
bool testExpression(const std::string &source, const std::string &expected)
{
    Lexer lexer(source);
    Parser parser(lexer);

    std::unique_ptr<Expr> expression = parser.parseArth();

    CodeGenerator generator;
    generator.generateExpression(*expression);

    std::string ir;
    llvm::raw_string_ostream output(ir);

    generator.printIR(output);
    output.flush();

    if (ir.find(expected) == std::string::npos)
    {
        std::cerr << "Code generation test failed: " << source << '\n';

        std::cerr << ir << '\n';
        return false;
    }

    return true;
}

int main()
{
    if (!testExpression("123", "ret i64 123"))
    {
        return 1;
    }

    if (!testExpression("10 + 20", "add i64 10, 20"))
    {
        return 1;
    }

    if (!testExpression("10 - 3", "sub i64 10, 3"))
    {
        return 1;
    }

    if (!testExpression("10 * 20", "mul i64 10, 20"))
    {
        return 1;
    }

    if (!testExpression("100 / 5", "sdiv i64 100, 5"))
    {
        return 1;
    }

    if (!testExpression("10 + 20 * 3", "mul i64 20, 3"))
    {
        return 1;
    }

    if (!testExpression("-10", "sub i64 0, 10"))
    {
        return 1;
    }

    if (!testExpression("-(10 + 20)", "sub i64 0, %addtmp"))
    {
        return 1;
    }

    if (!testExpression("--10", "sub i64 0, %negtmp"))
    {
        return 1;
    }

    // Test multiple print statements.
    {
        Lexer lexer(
            "print(123);"
            "print(10);"
            "print(10 + 20);"
        );

        Parser parser(lexer);

        std::vector<std::unique_ptr<Stmt>> program = parser.parseProgram();

        CodeGenerator generator;
        generator.generateProgram(program);

        std::string ir;
        llvm::raw_string_ostream output(ir);

        generator.printIR(output);
        output.flush();

        // Count the generated printf calls.
        const std::string callText = "call i32 (ptr, ...) @printf";

        std::size_t count = 0;
        std::size_t position = 0;

        while ((position = ir.find(callText, position)) != std::string::npos)
        {
            count++;
            position += callText.size();
        }

        if (count != 3 || ir.find("add i64 10, 20") == std::string::npos)
        {
            std::cerr << "Multiple print code generation failed!\n";
            std::cerr << ir << '\n';
            return 1;
        }
    }

    // Test integer variable declarations.
    {
        Lexer lexer(
            "int x = 10;"
            "int y = 20;"
            "print(x + y);"
        );

        Parser parser(lexer);

        std::vector<std::unique_ptr<Stmt>> program = parser.parseProgram();

        CodeGenerator generator;
        generator.generateProgram(program);

        std::string ir;
        llvm::raw_string_ostream output(ir);

        generator.printIR(output);
        output.flush();

        if (ir.find("alloca i64") == std::string::npos ||
            ir.find("store i64 10") == std::string::npos ||
            ir.find("store i64 20") == std::string::npos ||
            ir.find("load i64") == std::string::npos ||
            ir.find("add i64") == std::string::npos)
        {
            std::cerr << "Variable code generation test failed!\n";
            std::cerr << ir << '\n';
            return 1;
        }
    }

    // Test variable assignment code generation.
    {
        Lexer lexer(
            "int x = 10;"
            "x = x + 5;"
            "print(x);"
        );

        Parser parser(lexer);

        std::vector<std::unique_ptr<Stmt>> program = parser.parseProgram();

        CodeGenerator generator;
        generator.generateProgram(program);

        std::string ir;
        llvm::raw_string_ostream output(ir);

        generator.printIR(output);
        output.flush();

        std::size_t storeCount = 0;
        std::size_t position = 0;

        while ((position = ir.find("store i64", position)) != std::string::npos)
        {
            ++storeCount;
            position += 9;
        }

        if (storeCount != 2 || ir.find("add i64") == std::string::npos)
        {
            std::cerr << "Assignment code generation failed!\n";
            std::cerr << ir << '\n';
            return 1;
        }
    }

    std::cout << "All LLVM code generation tests passed!\n";

    return 0;
}
