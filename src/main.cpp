// The magic starts here.

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>

#include "lexer.hpp"
#include "token.hpp"

int main(int argc, char *argv[])
{
    // Check arguments.
    if (argc != 2)
    {
        std::cerr << "Usage: lito <file.lit>\n";
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
    Lexer lexer(std::move(source));

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
    std::cout << "Tokens: " << tokenCount << '\n';

    return 0;
}
