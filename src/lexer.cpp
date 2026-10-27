// The true code for lexer.

#include "lexer.hpp"

#include <cctype>
#include <cstddef>
#include <utility>

// Initialize the lexer with source code.
Lexer::Lexer(std::string source) : source_(std::move(source)) {}

// Calculate lexer progress.
std::size_t Lexer::progressPercent() const
{
    if (source_.empty())
    {
        return 100;
    }

    return static_cast<std::size_t>(100.0 * static_cast<double>(position_) /
                                    static_cast<double>(source_.size()));
}

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

// Look at char ahead without consuming.
char Lexer::peekNext() const
{
    if (position_ + 1 >= source_.size())
    {
        return '\0';
    }

    return source_[position_ + 1];
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
void Lexer::skipWhitespaceAndComments()
{
    while (!isAtEnd())
    {
        char current = peek();

        if (std::isspace(static_cast<unsigned char>(current)))
        {
            advance();
            continue;
        }

        // Skip single-line comments.
        if (current == '/' && peekNext() == '/')
        {
            while (!isAtEnd() && peek() != '\n')
            {
                advance();
            }

            continue;
        }

        break;
    }
}

// Read the next token from the source code.
Token Lexer::nextToken()
{
    // Skip single line comment.
    skipWhitespaceAndComments();

    // Skip multiline comments.
    while (peek() == '/' && peekNext() == '*')
    {
        const std::size_t commentLine = line_;
        const std::size_t commentColumn = column_;

        // Consume /*
        advance();
        advance();

        // Read until */
        while (!isAtEnd() && !(peek() == '*' && peekNext() == '/'))
        {
            advance();
        }

        // Throw Error: Unterminated comment.
        if (isAtEnd())
        {
            return {TokenType::Invalid, "Unterminated block comment",
                    commentLine, commentColumn};
        }

        // Consume */
        advance();
        advance();

        skipWhitespaceAndComments();
    }

    const std::size_t startLine = line_;
    const std::size_t startColumn = column_;

    if (isAtEnd())
    {
        return {TokenType::EndOfFile, "", startLine, startColumn};
    }

    char current = advance();

    // Integer handler.
    if (std::isdigit(static_cast<unsigned char>(current)))
    {
        std::string number(1, current);

        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek())))
        {
            number += advance();
        }

        return {TokenType::Integer, number, startLine, startColumn};
    }

    // Ident and keyword handler.
    if (std::isalpha(static_cast<unsigned char>(current)) || current == '_')
    {
        std::string identifier(1, current);

        while (!isAtEnd())
        {
            unsigned char next = static_cast<unsigned char>(peek());

            if (!std::isalnum(next) && next != '_')
            {
                break;
            }

            identifier += advance();
        }

        // Check reserved keywords.
        TokenType type = TokenType::Identifier;

        if (identifier == "def")
            type = TokenType::Def;
        else if (identifier == "return")
            type = TokenType::Return;
        else if (identifier == "if")
            type = TokenType::If;
        else if (identifier == "else")
            type = TokenType::Else;
        else if (identifier == "while")
            type = TokenType::While;
        else if (identifier == "int")
            type = TokenType::Int;
        else if (identifier == "str")
            type = TokenType::Str;
        else if (identifier == "bool")
            type = TokenType::Bool;

        return {type, identifier, startLine, startColumn};
    }

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

    // Strings
    case '"':
    {
        std::string literal = "\"";

        while (!isAtEnd() && peek() != '"' && peek() != '\n')
        {
            // Escape sequences are not supported yet.
            if (peek() == '\\')
            {
                return {TokenType::Invalid, literal + "\\", startLine,
                        startColumn};
            }

            literal += advance();
        }

        // Check for unterminated strings.
        if (isAtEnd() || peek() == '\n')
        {
            return {TokenType::Invalid, literal, startLine, startColumn};
        }

        // Consume the closing quotation mark.
        literal += advance();

        return {TokenType::String, literal, startLine, startColumn};
    }

    default:
        return {TokenType::Invalid, std::string(1, current), startLine,
                startColumn};
    }
}
