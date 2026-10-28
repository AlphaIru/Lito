// LLVM integer code generation tests.

#include <iostream>
#include <memory>
#include <string>

#include "codegen.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <llvm/Support/raw_ostream.h>

// Check the LLVM IR generated from a Lito expression.
bool testExpression(const std::string &source, const std::string &expected)
{
    Lexer lexer(source);
    Parser parser(lexer);

    std::unique_ptr<Expr> expression = parser.parseArth();

    CodeGenerator generator;
    generator.generateExpression(*expression);

    std::string ir;
    llvm::raw_string_ostream output(ir);

    generator.printIR(output);
    output.flush();

    if (ir.find(expected) == std::string::npos)
    {
        std::cerr << "Code generation test failed: " << source << '\n';

        std::cerr << ir << '\n';
        return false;
    }

    return true;
}

int main()
{
    if (!testExpression("123", "ret i64 123"))
    {
        return 1;
    }

    if (!testExpression("10 + 20", "add i64 10, 20"))
    {
        return 1;
    }

    if (!testExpression("10 - 3", "sub i64 10, 3"))
    {
        return 1;
    }

    if (!testExpression("10 * 20", "mul i64 10, 20"))
    {
        return 1;
    }

    if (!testExpression("100 / 5", "sdiv i64 100, 5"))
    {
        return 1;
    }

    if (!testExpression("10 + 20 * 3", "mul i64 20, 3"))
    {
        return 1;
    }

    if (!testExpression("-10", "sub i64 0, 10"))
    {
        return 1;
    }

    if (!testExpression("-(10 + 20)", "sub i64 0, %addtmp"))
    {
        return 1;
    }

    if (!testExpression("--10", "sub i64 0, %negtmp"))
    {
        return 1;
    }

    // Test print() code generation.
    {
        Lexer lexer("print(123);");
        Parser parser(lexer);

        std::unique_ptr<Stmt> statement = parser.parseStatement();

        ExprStmt *exprStmt = dynamic_cast<ExprStmt *>(statement.get());

        if (exprStmt == nullptr)
        {
            std::cerr << "Expected ExprStmt!\n";
            return 1;
        }

        CallExpr *call = dynamic_cast<CallExpr *>(exprStmt->expression.get());

        if (call == nullptr)
        {
            std::cerr << "Expected CallExpr!\n";
            return 1;
        }

        CodeGenerator generator;
        generator.generatePrint(*call);

        std::string ir;
        llvm::raw_string_ostream output(ir);

        generator.printIR(output);
        output.flush();

        if (ir.find("@printf") == std::string::npos ||
            ir.find("i64 123") == std::string::npos)
        {
            std::cerr << "Print code generation failed!\n";
            return 1;
        }
    }

    std::cout << "All LLVM code generation tests passed!\n";

    return 0;
}
