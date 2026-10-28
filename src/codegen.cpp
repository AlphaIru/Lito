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

    llvm::Function *function = llvm::Function::Create(
        functionType,
        llvm::Function::ExternalLinkage,
        "lito_expression",
        module_
    );

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
        mainType, llvm::Function::ExternalLinkage, "main", module_
    );

    llvm::BasicBlock *mainEntry =
        llvm::BasicBlock::Create(context_, "entry", mainFunction);

    // Switch to the main function.
    builder_.SetInsertPoint(mainEntry);

    // Call the generated expression.
    llvm::Value *value = builder_.CreateCall(
        function->getFunctionType(), function, {}, "result"
    );

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

// Generate LLVM IR for the entire program.
void CodeGenerator::generateProgram(
    const std::vector<std::unique_ptr<Stmt>> &program
)
{
    // Create main().
    llvm::FunctionType *mainType =
        llvm::FunctionType::get(builder_.getInt32Ty(), false);

    llvm::Function *mainFunction = llvm::Function::Create(
        mainType, llvm::Function::ExternalLinkage, "main", module_
    );

    llvm::BasicBlock *entry =
        llvm::BasicBlock::Create(context_, "entry", mainFunction);

    builder_.SetInsertPoint(entry);

    // Declare print().
    llvm::Type *printfArguments[] = {builder_.getPtrTy()};

    llvm::FunctionType *printfType =
        llvm::FunctionType::get(builder_.getInt32Ty(), printfArguments, true);

    llvm::FunctionCallee printfFunction =
        module_.getOrInsertFunction("printf", printfType);

    // Create the integer format string.
    llvm::GlobalVariable *format = builder_.CreateGlobalString("%lld\n", "fmt");

    // Generate code for each statement.
    for (const std::unique_ptr<Stmt> &statement : program)
    {
        // Variable declaration.
        const VarDeclStmt *declaration =
            dynamic_cast<const VarDeclStmt *>(statement.get());

        if (declaration != nullptr)
        {
            // Reject duplicate variable declarations.
            if (variables_.find(declaration->name) != variables_.end())
            {
                throw std::runtime_error(
                    "Variable already declared: " + declaration->name
                );
            }

            // Generate the initializer expression.
            llvm::Value *initialValue =
                emitExpression(*declaration->initializer);

            // Allocate memory for the variable.
            llvm::AllocaInst *variable = builder_.CreateAlloca(
                builder_.getInt64Ty(), nullptr, declaration->name
            );

            // Store the initial value.
            builder_.CreateStore(initialValue, variable);

            // Register the variable.
            variables_.emplace(declaration->name, variable);

            continue;
        }

        // Assignment statement.
        const AssignStmt *assignment =
            dynamic_cast<const AssignStmt *>(statement.get());

        if (assignment != nullptr)
        {
            // Find the existing variable.
            std::unordered_map<std::string, llvm::AllocaInst *>::const_iterator
                variable = variables_.find(assignment->name);

            if (variable == variables_.end())
            {
                throw std::runtime_error(
                    "Undefined variable: " + assignment->name
                );
            }

            // Evaluate the new value.
            llvm::Value *value = emitExpression(*assignment->value);

            // Update the variable.
            builder_.CreateStore(value, variable->second);

            continue;
        }

        const ExprStmt *exprStmt =
            dynamic_cast<const ExprStmt *>(statement.get());

        if (exprStmt == nullptr)
        {
            throw std::runtime_error("Unsupported statement");
        }

        const CallExpr *call =
            dynamic_cast<const CallExpr *>(exprStmt->expression.get());

        if (call == nullptr)
        {
            throw std::runtime_error("Expected a function call");
        }

        const IdentifierExpr *callee =
            dynamic_cast<const IdentifierExpr *>(call->callee.get());

        if (callee == nullptr || callee->name != "print")
        {
            throw std::runtime_error("Unknown function");
        }

        if (call->arguments.size() != 1)
        {
            throw std::runtime_error("print() expects exactly one argument");
        }

        // Generate the argument expression.
        llvm::Value *value = emitExpression(*call->arguments[0]);

        // Print the result.
        builder_.CreateCall(printfFunction, {format, value});
    }

    // Finish main().
    builder_.CreateRet(builder_.getInt32(0));

    // Verify the module.
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
