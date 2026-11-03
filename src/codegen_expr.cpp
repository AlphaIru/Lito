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

    // Floating-point literal.
    const FloatExpr *floating = dynamic_cast<const FloatExpr *>(&expression);

    if (floating != nullptr)
    {
        double number = std::stod(floating->value);

        return llvm::ConstantFP::get(builder_.getDoubleTy(), number);
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
            if (operand->getType()->isIntegerTy(64))
            {
                return builder_.CreateNeg(operand, "negtmp");
            }

            if (operand->getType()->isDoubleTy())
            {
                return builder_.CreateFNeg(operand, "fnegtmp");
            }

            throw std::runtime_error("Unsupported operand type");

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

        // Both operands must have the same type.
        if (left->getType() != right->getType())
        {
            throw std::runtime_error("Type mismatch in binary expression");
        }

        const bool isInteger = left->getType()->isIntegerTy(64);

        const bool isFloat = left->getType()->isDoubleTy();

        if (!isInteger && !isFloat)
        {
            throw std::runtime_error("Unsupported binary operand type");
        }

        switch (binary->op)
        {
        case TokenType::Plus:
            if (isFloat)
            {
                return builder_.CreateFAdd(left, right, "faddtmp");
            }
            return builder_.CreateAdd(left, right, "addtmp");

        case TokenType::Minus:
            if (isFloat)
            {
                return builder_.CreateFSub(left, right, "fsubtmp");
            }
            return builder_.CreateSub(left, right, "subtmp");

        case TokenType::Star:
            if (isFloat)
            {
                return builder_.CreateFMul(left, right, "fmultmp");
            }
            return builder_.CreateMul(left, right, "multmp");

        case TokenType::Slash:
            if (isFloat)
            {
                return builder_.CreateFDiv(left, right, "fdivtmp");
            }
            return builder_.CreateSDiv(left, right, "divtmp");

        default:
            throw std::runtime_error("Unsupported binary operator");
        }
    }

    throw std::runtime_error("Unsupported expression");
}
