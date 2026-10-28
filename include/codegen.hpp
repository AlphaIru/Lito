// Creates the bytecode from source.

#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

// Test purpose
#include <llvm/IR/NoFolder.h>

#include "ast.hpp"

class CodeGenerator
{
  public:
    CodeGenerator();

    void generateExpression(const Expr &expression);
    void generatePrint(const CallExpr &call);

    void printIR(llvm::raw_ostream &output) const;

  private:
    llvm::LLVMContext context_;
    llvm::Module module_;
    llvm::IRBuilder<llvm::NoFolder> builder_;

    llvm::Value *emitExpression(const Expr &expression);
};
