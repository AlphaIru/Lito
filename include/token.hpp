// This handles the token for the Lito.

#pragma once

#include <cstddef>
#include <string>

// Types for Lito
enum class TokenType
{
    // Literals
    Identifier,
    Integer,
    String,

    // Keywords
    Def,
    Return,
    If,
    Else,
    While,
    Int,
    Str,
    Bool,

    // Operators
    Plus,
    Minus,
    Star,
    Slash,
    Assign,
    Equal,
    Arrow,

    // Symbols
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    LeftBracket,
    RightBracket,
    Semicolon,
    Comma,
    Colon,

    // Specials
    EndOfFile,
    Invalid
};

struct Token
{
    TokenType type;
    std::string text;
    std::size_t line;
    std::size_t column;
};
