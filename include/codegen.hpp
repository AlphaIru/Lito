// Creates the bytecode from source.

#pragma once

#include <llvm/IR/Instructions.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instruction.h>
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
    void generateProgram(const std::vector<std::unique_ptr<Stmt>> &program);

    void printIR(llvm::raw_ostream &output) const;

  private:
    llvm::LLVMContext context_;
    llvm::Module module_;
    llvm::IRBuilder<llvm::NoFolder> builder_;

    llvm::Value *emitExpression(const Expr &expression);

    std::unordered_map<std::string, llvm::AllocaInst *> variables_;
};
