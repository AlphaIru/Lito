#include <iostream>
#include <string>
#include <vector>

#include "lexer.hpp"

// Check if the lexer works with expected tokens.

bool testLexer(const std::string &source,
               const std::vector<TokenType> &expected)
{
    Lexer lexer(source);

    for (std::size_t i = 0; i < expected.size(); i++)
    {
        Token token = lexer.nextToken();

        if (token.type != expected[i])
        {
            std::cerr << "Test failed at token " << i
                      << " (Line: " << token.line
                      << ", Column: " << token.column << ")\n";
            return false;
        }
    }

    return true;
}

int main()
{
    // Test 1: Variable declaration
    if (!testLexer("int x = 10;", {TokenType::Int, TokenType::Identifier,
                                   TokenType::Assign, TokenType::Integer,
                                   TokenType::Semicolon, TokenType::EndOfFile}))
    {
        return 1;
    }

    // Test 2: Comments
    if (!testLexer("// Hello\n/* World */ 42",
                   {TokenType::Integer, TokenType::EndOfFile}))
    {
        return 1;
    }

    // Test 3: Unterminated block comment
    if (!testLexer("/* Missing end", {TokenType::Invalid}))
    {
        return 1;
    }

    std::cout << "All lexer tests passed!\n";
    return 0;
}
