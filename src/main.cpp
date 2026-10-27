// The magic starts here.

#include <iostream>
#include <string>

#include "lexer.hpp"

int main()
{
    // Sample Code.
    std::string source = "{ ( ); } + - * / = == ->;";

    // Initialize lexer
    Lexer lexer(source);

    // Read until EOF.
    while (true)
    {
        Token token = lexer.nextToken();

        if (token.type == TokenType::EndOfFile)
        {
            std::cout << "EOF\n";
            break;
        }

        if (token.type == TokenType::Invalid)
        {
            std::cerr << "Invalid token: " << token.text << '\n';
            return 1;
        }

        std::cout << "Token: " << token.text << " (Line: " << token.line
                  << ", Column: " << token.column << ")\n";
    }

    return 0;
}
