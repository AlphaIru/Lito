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
    char advance();

    // Ignores spaces, tabs, and newlines.
    void skipWhitespace();
};
