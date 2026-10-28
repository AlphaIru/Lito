// AST... Oh f---ing god...

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "token.hpp"

// Base class for all expressions.
struct Expr
{
    virtual ~Expr() = default;
};

// Base class for all statements.
struct Stmt
{
    virtual ~Stmt() = default;
};

// Expr stmt.
struct ExprStmt : Stmt
{
    std::unique_ptr<Expr> expression;
};

// Variable declaration statement.
struct VarDeclStmt : Stmt
{
    std::string name;
    std::unique_ptr<Expr> initializer;
};

// Assignment statement.
struct AssignStmt : Stmt
{
    std::string name;
    std::unique_ptr<Expr> value;
};

// Call exp.
struct CallExpr : Expr
{
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> arguments;
};

// Integer literal exp.
struct IntegerExpr : Expr
{
    std::string value;

    explicit IntegerExpr(std::string value) : value(std::move(value))
    {
    }
};

// Identifier exp.
struct IdentifierExpr : Expr
{
    std::string name;

    explicit IdentifierExpr(std::string name) : name(std::move(name))
    {
    }
};

// Binary oper exp.
struct BinaryExpr : Expr
{
    std::unique_ptr<Expr> left;
    TokenType op;
    std::unique_ptr<Expr> right;
};

// Unary oper expr.
struct UnaryExpr : Expr
{
    TokenType op;
    std::unique_ptr<Expr> operand;
};
