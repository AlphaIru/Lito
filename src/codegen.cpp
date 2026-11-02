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
        emitStatement(*statement, printfFunction, format);
    }

    // Finish main().
    builder_.CreateRet(builder_.getInt32(0));

    // Verify the module.
    if (llvm::verifyModule(module_, &llvm::errs()))
    {
        throw std::runtime_error("Invalid LLVM module");
    }
}

// Print generated LLVM IR.
void CodeGenerator::printIR(llvm::raw_ostream &output) const
{
    module_.print(output, nullptr);
}
