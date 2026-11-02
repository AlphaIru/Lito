// Lito LLVM expression code generator.

#include "codegen.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>

// Generate LLVM values from AST expressions.
llvm::Value *CodeGenerator::emitExpression(const Expr &expression)
{
    // Integer literal.
    const IntegerExpr *integer = dynamic_cast<const IntegerExpr *>(&expression);

    if (integer != nullptr)
    {
        std::int64_t number = std::stoll(integer->value);

        return llvm::ConstantInt::getSigned(builder_.getInt64Ty(), number);
    }

    // Identifier expression.
    const IdentifierExpr *identifier =
        dynamic_cast<const IdentifierExpr *>(&expression);

    if (identifier != nullptr)
    {
        std::unordered_map<std::string, llvm::AllocaInst *>::const_iterator
            variable = variables_.find(identifier->name);

        if (variable == variables_.end())
        {
            throw std::runtime_error("Undefined variable: " + identifier->name);
        }

        return builder_.CreateLoad(
            builder_.getInt64Ty(), variable->second, identifier->name + ".value"
        );
    }

    // Unary expression.
    const UnaryExpr *unary = dynamic_cast<const UnaryExpr *>(&expression);

    if (unary != nullptr)
    {
        llvm::Value *operand = emitExpression(*unary->operand);

        switch (unary->op)
        {
        case TokenType::Minus:
            return builder_.CreateNeg(operand, "negtmp");

        default:
            throw std::runtime_error("Unsupported unary operator");
        }
    }

    // Binary expression.
    const BinaryExpr *binary = dynamic_cast<const BinaryExpr *>(&expression);

    if (binary != nullptr)
    {
        llvm::Value *left = emitExpression(*binary->left);

        llvm::Value *right = emitExpression(*binary->right);

        switch (binary->op)
        {
        case TokenType::Plus:
            return builder_.CreateAdd(left, right, "addtmp");

        case TokenType::Minus:
            return builder_.CreateSub(left, right, "subtmp");

        case TokenType::Star:
            return builder_.CreateMul(left, right, "multmp");

        case TokenType::Slash:
            return builder_.CreateSDiv(left, right, "divtmp");

        default:
            throw std::runtime_error("Unsupported binary operator");
        }
    }

    throw std::runtime_error("Unsupported expression");
}
