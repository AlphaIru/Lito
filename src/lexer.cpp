// The true code for lexer.

#include "lexer.hpp"

#include <cctype>
#include <utility>

// Initialize the lexer with source code.
Lexer::Lexer(std::string source) : source_(std::move(source)) {}

// Check if it is a end of the source.
bool Lexer::isAtEnd() const { return position_ >= source_.size(); }

// Look at the current char.
char Lexer::peek() const
{
    if (isAtEnd())
    {
        return '\0';
    }

    return source_[position_];
}

// Consume the current char and move foward.
char Lexer::advance()
{
    if (isAtEnd())
    {
        return '\0';
    }

    char current = source_[position_++];

    if (current == '\n')
    {
        line_++;
        column_ = 1;
    }
    else
    {
        column_++;
    }

    return current;
}

// Skips whitespaces.
void Lexer::skipWhitespace()
{
    while (!isAtEnd())
    {
        unsigned char curent = static_cast<unsigned char>(peek());

        if (!std::isspace(curent))
        {
            break;
        }

        advance();
    }
}

// Read the next token from the source code.
Token Lexer::nextToken()
{
    skipWhitespace();

    const std::size_t startLine = line_;
    const std::size_t startColumn = column_;

    if (isAtEnd())
    {
        return {TokenType::EndOfFile, "", startLine, startColumn};
    }

    char current = advance();

    switch (current)
    {
    case '(':
        return {TokenType::LeftParen, "(", startLine, startColumn};

    case ')':
        return {TokenType::RightParen, ")", startLine, startColumn};

    case '{':
        return {TokenType::LeftBrace, "{", startLine, startColumn};

    case '}':
        return {TokenType::RightBrace, "}", startLine, startColumn};

    case '[':
        return {TokenType::LeftBracket, "[", startLine, startColumn};

    case ']':
        return {TokenType::RightBracket, "]", startLine, startColumn};

    case ';':
        return {TokenType::Semicolon, ";", startLine, startColumn};

    case ',':
        return {TokenType::Comma, ",", startLine, startColumn};

    case ':':
        return {TokenType::Colon, ":", startLine, startColumn};

    // Arithmetic operators
    case '+':
        return {TokenType::Plus, "+", startLine, startColumn};

    case '-':
        if (peek() == '>')
        {
            advance();
            return {TokenType::Arrow, "->", startLine, startColumn};
        }
        return {TokenType::Minus, "-", startLine, startColumn};

    case '*':
        return {TokenType::Star, "*", startLine, startColumn};

    case '/':
        return {TokenType::Slash, "/", startLine, startColumn};

    // Assignment and equality
    case '=':
        if (peek() == '=')
        {
            advance();
            return {TokenType::Equal, "==", startLine, startColumn};
        }
        return {TokenType::Assign, "=", startLine, startColumn};

    default:
        return {TokenType::Invalid, std::string(1, current), startLine,
                startColumn};
    }
}
