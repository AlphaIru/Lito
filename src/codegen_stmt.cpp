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

        // Check variable type.
        if (declaration->type != TypeKind::Int)
        {
            throw std::runtime_error("Float variables are not supported yet");
        }

        if (variables_.find(declaration->name) != variables_.end())
        {
            throw std::runtime_error(
                "Variable already declared: " + declaration->name
            );
        }

        llvm::Value *initialValue = emitExpression(*declaration->initializer);

        if (!initialValue->getType()->isIntegerTy(64))
        {
            throw std::runtime_error(
                "Integer variable requires an integer initializer"
            );
        }

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

        if (!value->getType()->isIntegerTy(64))
        {
            throw std::runtime_error(
                "Integer variable requires an integer value"
            );
        }

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

    // Print integer values.
    if (value->getType()->isIntegerTy(64))
    {
        builder_.CreateCall(printfFunction, {format, value});

        return;
    }

    // Print floating-point values.
    if (value->getType()->isDoubleTy())
    {
        llvm::GlobalVariable *floatFormat =
            builder_.CreateGlobalString("%.6f\n", "fmt.float");

        builder_.CreateCall(printfFunction, {floatFormat, value});

        return;
    }

    throw std::runtime_error("Unsupported print argument type");
}
