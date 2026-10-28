// Lito LLVM code generator.

#include <cstdint>
#include <stdexcept>
#include <string>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>

#include "ast.hpp"
#include "codegen.hpp"
#include "token.hpp"

// Initialize LLVM components.
CodeGenerator::CodeGenerator()
    : context_(), module_("lito", context_), builder_(context_)
{
    // Configure the target architecture.
    llvm::Triple target(llvm::sys::getDefaultTargetTriple());

    module_.setTargetTriple(target);
}

// Generate LLVM IR for an expression.
void CodeGenerator::generateExpression(const Expr &expression)
{
    // Create the Lito expression function.
    llvm::FunctionType *functionType =
        llvm::FunctionType::get(builder_.getInt64Ty(), false);

    llvm::Function *function =
        llvm::Function::Create(functionType, llvm::Function::ExternalLinkage,
                               "lito_expression", module_);

    llvm::BasicBlock *entry =
        llvm::BasicBlock::Create(context_, "entry", function);

    builder_.SetInsertPoint(entry);

    // Generate expression.
    llvm::Value *result = emitExpression(expression);

    // Finish the Lito expression function.
    builder_.CreateRet(result);

    // Create the native entry point.
    llvm::FunctionType *mainType =
        llvm::FunctionType::get(builder_.getInt32Ty(), false);

    llvm::Function *mainFunction = llvm::Function::Create(
        mainType, llvm::Function::ExternalLinkage, "main", module_);

    llvm::BasicBlock *mainEntry =
        llvm::BasicBlock::Create(context_, "entry", mainFunction);

    // Switch to the main function.
    builder_.SetInsertPoint(mainEntry);

    // Call the generated expression.
    llvm::Value *value = builder_.CreateCall(function->getFunctionType(),
                                             function, {}, "result");

    // Declare printf().
    llvm::Type *printfArguments[] = {builder_.getPtrTy()};

    llvm::FunctionType *printfType =
        llvm::FunctionType::get(builder_.getInt32Ty(), printfArguments, true);

    llvm::FunctionCallee printfFunction =
        module_.getOrInsertFunction("printf", printfType);

    // Create the format string.
    llvm::GlobalVariable *format = builder_.CreateGlobalString("%lld\n", "fmt");

    // Print the expression result.
    builder_.CreateCall(printfFunction, {format, value});

    // Finish main().
    builder_.CreateRet(builder_.getInt32(0));

    // Verify the LLVM module.
    if (llvm::verifyModule(module_, &llvm::errs()))
    {
        throw std::runtime_error("Invalid LLVM module");
    }
}

// Generate a native program for print().
void CodeGenerator::generatePrint(const CallExpr &call)
{
    // Check the function name.
    const IdentifierExpr *callee =
        dynamic_cast<const IdentifierExpr *>(call.callee.get());

    if (callee == nullptr || callee->name != "print")
    {
        throw std::runtime_error("Unknown function");
    }

    // Currently, print() accepts one argument.
    if (call.arguments.size() != 1)
    {
        throw std::runtime_error("print() expects exactly one argument");
    }

    // Create main().
    llvm::FunctionType *mainType =
        llvm::FunctionType::get(builder_.getInt32Ty(), false);

    llvm::Function *mainFunction = llvm::Function::Create(
        mainType, llvm::Function::ExternalLinkage, "main", module_);

    llvm::BasicBlock *entry =
        llvm::BasicBlock::Create(context_, "entry", mainFunction);

    builder_.SetInsertPoint(entry);

    // Evaluate the print argument.
    llvm::Value *value = emitExpression(*call.arguments[0]);

    // Declare printf().
    llvm::Type *printfArguments[] = {builder_.getPtrTy()};

    llvm::FunctionType *printfType =
        llvm::FunctionType::get(builder_.getInt32Ty(), printfArguments, true);

    llvm::FunctionCallee printfFunction =
        module_.getOrInsertFunction("printf", printfType);

    // Create the integer format string.
    llvm::GlobalVariable *format = builder_.CreateGlobalString("%lld\n", "fmt");

    // Call printf().
    builder_.CreateCall(printfFunction, {format, value});

    // Return success from main().
    builder_.CreateRet(builder_.getInt32(0));

    // Verify the generated module.
    if (llvm::verifyModule(module_, &llvm::errs()))
    {
        throw std::runtime_error("Invalid LLVM module");
    }
}

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

// Print generated LLVM IR.
void CodeGenerator::printIR(llvm::raw_ostream &output) const
{
    module_.print(output, nullptr);
}
