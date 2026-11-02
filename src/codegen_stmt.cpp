// Lito LLVM statement code generator.

#include "codegen.hpp"

#include <stdexcept>
#include <string>
#include <unordered_map>

#include <llvm/IR/Instructions.h>

// Generate LLVM IR for a statement.
void CodeGenerator::emitStatement(
    const Stmt &statement,
    llvm::FunctionCallee printfFunction,
    llvm::Value *format
)
{
    // Variable declaration.
    const VarDeclStmt *declaration =
        dynamic_cast<const VarDeclStmt *>(&statement);

    if (declaration != nullptr)
    {
        if (variables_.find(declaration->name) != variables_.end())
        {
            throw std::runtime_error(
                "Variable already declared: " + declaration->name
            );
        }

        llvm::Value *initialValue = emitExpression(*declaration->initializer);

        llvm::AllocaInst *variable = builder_.CreateAlloca(
            builder_.getInt64Ty(), nullptr, declaration->name
        );

        builder_.CreateStore(initialValue, variable);

        variables_.emplace(declaration->name, variable);

        return;
    }

    // Assignment statement.
    const AssignStmt *assignment = dynamic_cast<const AssignStmt *>(&statement);

    if (assignment != nullptr)
    {
        std::unordered_map<std::string, llvm::AllocaInst *>::const_iterator
            variable = variables_.find(assignment->name);

        if (variable == variables_.end())
        {
            throw std::runtime_error("Undefined variable: " + assignment->name);
        }

        llvm::Value *value = emitExpression(*assignment->value);

        builder_.CreateStore(value, variable->second);

        return;
    }

    // Expression statement.
    const ExprStmt *exprStmt = dynamic_cast<const ExprStmt *>(&statement);

    if (exprStmt == nullptr)
    {
        throw std::runtime_error("Unsupported statement");
    }

    // Currently, expression statements must call print().
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

    llvm::Value *value = emitExpression(*call->arguments[0]);

    builder_.CreateCall(printfFunction, {format, value});
}
