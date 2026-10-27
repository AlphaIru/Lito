// This handles the lexer fucntions for Lito.

#pragma once

#include <cstddef>
#include <string>

#include "token.hpp"

// Converts Lito codes into tokens.
class Lexer
{
  public:
    explicit Lexer(std::string source);

    // Returns the percentage of source code.
    std::size_t progressPercent() const;

    Token nextToken();

  private:
    // The source code
    std::string source_;

    // Position of that source code.
    std::size_t position_ = 0;
    std::size_t line_ = 1;
    std::size_t column_ = 1;

    // Character handling.
    bool isAtEnd() const;
    char peek() const;
    char peekNext() const;
    char advance();

    // Ignores whitespace and comments.
    void skipWhitespaceAndComments();
};
