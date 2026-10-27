// Lito parser implementation.

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "parser.hpp"

Parser::Parser(Lexer &lexer) : lexer_(lexer), current_(lexer_.nextToken()) {}

void Parser::advance() { current_ = lexer_.nextToken(); }

IntegerExpr Parser::parseInteger()
{
    if (current_.type != TokenType::Integer)
    {
        throw std::runtime_error(
            "Expected an integer at line: " + std::to_string(current_.line) +
            ", column: " + std::to_string(current_.column));
    }

    IntegerExpr result{current_.text};

    advance();

    return result;
}

// Parse an expr stmt.
std::unique_ptr<Stmt> Parser::parseStatement()
{
    // Parse the expression.
    std::unique_ptr<Expr> expression = parseArth();

    // Expect a semicolon.
    if (current_.type != TokenType::Semicolon)
    {
        throw std::runtime_error("Expected ';' at line " +
                                 std::to_string(current_.line) + ", column " +
                                 std::to_string(current_.column));
    }

    // Consume ';'
    advance();

    // Create an expression statement.
    auto statement = std::make_unique<ExprStmt>();
    statement->expression = std::move(expression);

    return statement;
}

// Parse a primary expr.
std::unique_ptr<Expr> Parser::parsePrimary()
{
    // Identifier.
    if (current_.type == TokenType::Identifier)
    {
        std::string name = current_.text;
        advance();

        return std::make_unique<IdentifierExpr>(std::move(name));
    }

    // Integer literal.
    if (current_.type == TokenType::Integer)
    {
        return std::make_unique<IntegerExpr>(parseInteger());
    }

    // Parenthesized expression.
    if (current_.type == TokenType::LeftParen)
    {
        // Consume '('
        advance();

        std::unique_ptr<Expr> expression = parseArth();

        if (current_.type != TokenType::RightParen)
        {
            throw std::runtime_error(
                "Expected ')' at line " + std::to_string(current_.line) +
                ", column " + std::to_string(current_.column));
        }

        // Consume ')'
        advance();

        return expression;
    }

    throw std::runtime_error("Unexpected token at line " +
                             std::to_string(current_.line) + ", column " +
                             std::to_string(current_.column));
}

// Parse Expr.
std::unique_ptr<Expr> Parser::parseArth()
{
    std::unique_ptr<Expr> left = parseMult();

    while (current_.type == TokenType::Plus ||
           current_.type == TokenType::Minus)
    {
        TokenType op = current_.type;
        advance();

        std::unique_ptr<Expr> right = parseMult();

        auto binary = std::make_unique<BinaryExpr>();

        binary->left = std::move(left);
        binary->op = op;
        binary->right = std::move(right);

        left = std::move(binary);
    }

    return left;
}

// Parse multiplication and division.
std::unique_ptr<Expr> Parser::parseMult()
{
    std::unique_ptr<Expr> left = parseUnary();

    while (current_.type == TokenType::Star ||
           current_.type == TokenType::Slash)
    {
        TokenType op = current_.type;
        advance();

        std::unique_ptr<Expr> right = parseUnary();

        auto binary = std::make_unique<BinaryExpr>();

        binary->left = std::move(left);
        binary->op = op;
        binary->right = std::move(right);

        left = std::move(binary);
    }

    return left;
}

// Parse unary expr.
std::unique_ptr<Expr> Parser::parseUnary()
{
    if (current_.type == TokenType::Minus)
    {
        TokenType op = current_.type;
        advance();

        auto unary = std::make_unique<UnaryExpr>();

        unary->op = op;
        unary->operand = parseUnary();

        return unary;
    }

    return parseCall();
}

// Parse call expr.
std::unique_ptr<Expr> Parser::parseCall()
{
    std::unique_ptr<Expr> expression = parsePrimary();

    while (current_.type == TokenType::LeftParen)
    {
        // Consume '('
        advance();

        auto call = std::make_unique<CallExpr>();
        call->callee = std::move(expression);

        // Parse arguments.
        if (current_.type != TokenType::RightParen)
        {
            do
            {
                call->arguments.push_back(parseArth());

                if (current_.type != TokenType::Comma)
                {
                    break;
                }

                // Consume ','
                advance();
            } while (true);
        }

        if (current_.type != TokenType::RightParen)
        {
            throw std::runtime_error("Expected ')' after function arguments");
        }

        // Consume ')'
        advance(); // Consume ')'

        expression = std::move(call);
    }

    return expression;
}
