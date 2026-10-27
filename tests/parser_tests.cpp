// Unit tests for the Lito parser.

#include <iostream>
#include <string>

#include "parser.hpp"

int main()
{
    // Test 1: Integer expression.
    {
        Lexer lexer("123");
        Parser parser(lexer);

        IntegerExpr result = parser.parseInteger();

        if (result.value != "123")
        {
            std::cerr << "Integer expression test failed!\n";
            return 1;
        }
    }

    // Test 2: Binary expression (10 + 20).
    {
        Lexer lexer("10 + 20");
        Parser parser(lexer);

        auto expression = parser.parseArth();

        auto *binary = dynamic_cast<BinaryExpr *>(expression.get());

        if (!binary || binary->op != TokenType::Plus)
        {
            std::cerr << "Binary expression test failed!\n";
            return 1;
        }

        auto *left = dynamic_cast<IntegerExpr *>(binary->left.get());

        auto *right = dynamic_cast<IntegerExpr *>(binary->right.get());

        if (!left || !right || left->value != "10" || right->value != "20")
        {
            std::cerr << "Binary operands test failed!\n";
            return 1;
        }
    }

    // Test 3: Operator precedence (10 + 20 * 3).
    {
        Lexer lexer("10 + 20 * 3");
        Parser parser(lexer);

        auto expression = parser.parseArth();

        auto *addition = dynamic_cast<BinaryExpr *>(expression.get());

        if (!addition || addition->op != TokenType::Plus)
        {
            std::cerr << "Addition precedence test failed!\n";
            return 1;
        }

        auto *multiplication =
            dynamic_cast<BinaryExpr *>(addition->right.get());

        if (!multiplication || multiplication->op != TokenType::Star)
        {
            std::cerr << "Multiplication precedence test failed!\n";
            return 1;
        }

        auto *left = dynamic_cast<IntegerExpr *>(multiplication->left.get());

        auto *right = dynamic_cast<IntegerExpr *>(multiplication->right.get());

        if (!left || !right || left->value != "20" || right->value != "3")
        {
            std::cerr << "Multiplication operands test failed!\n";
            return 1;
        }
    }

    // Test 4: Parentheses (10 + 20) * 3.
    {
        Lexer lexer("(10 + 20) * 3");
        Parser parser(lexer);

        auto expression = parser.parseArth();

        auto *multiplication = dynamic_cast<BinaryExpr *>(expression.get());

        if (!multiplication || multiplication->op != TokenType::Star)
        {
            std::cerr << "Parentheses test failed!\n";
            return 1;
        }

        auto *addition = dynamic_cast<BinaryExpr *>(multiplication->left.get());

        auto *right = dynamic_cast<IntegerExpr *>(multiplication->right.get());

        if (!addition || addition->op != TokenType::Plus || !right ||
            right->value != "3")
        {
            std::cerr << "Parentheses structure test failed!\n";
            return 1;
        }
    }

    // Test 5: Unary minus (-10 + 20).
    {
        Lexer lexer("-10 + 20");
        Parser parser(lexer);

        auto expression = parser.parseArth();

        auto *addition = dynamic_cast<BinaryExpr *>(expression.get());

        if (!addition || addition->op != TokenType::Plus)
        {
            std::cerr << "Unary addition test failed!\n";
            return 1;
        }

        auto *unary = dynamic_cast<UnaryExpr *>(addition->left.get());

        if (!unary || unary->op != TokenType::Minus)
        {
            std::cerr << "Unary minus test failed!\n";
            return 1;
        }

        auto *integer = dynamic_cast<IntegerExpr *>(unary->operand.get());

        if (!integer || integer->value != "10")
        {
            std::cerr << "Unary operand test failed!\n";
            return 1;
        }
    }

    // Test 6: Identifier expression.
    {
        Lexer lexer("print");
        Parser parser(lexer);

        auto expression = parser.parseArth();

        auto *identifier = dynamic_cast<IdentifierExpr *>(expression.get());

        if (!identifier || identifier->name != "print")
        {
            std::cerr << "Identifier expression test failed!\n";
            return 1;
        }
    }

    // Test 7: Function call expression.
    {
        Lexer lexer("print(123)");
        Parser parser(lexer);

        auto expression = parser.parseArth();

        auto *call = dynamic_cast<CallExpr *>(expression.get());

        if (!call || call->arguments.size() != 1)
        {
            std::cerr << "Function call test failed!\n";
            return 1;
        }

        auto *callee = dynamic_cast<IdentifierExpr *>(call->callee.get());

        auto *argument = dynamic_cast<IntegerExpr *>(call->arguments[0].get());

        if (!callee || callee->name != "print" || !argument ||
            argument->value != "123")
        {
            std::cerr << "Function call structure test failed!\n";
            return 1;
        }
    }

    // Test 8: Expression statement.
    {
        Lexer lexer("print(123);");
        Parser parser(lexer);

        auto statement = parser.parseStatement();

        auto *exprStmt = dynamic_cast<ExprStmt *>(statement.get());

        if (!exprStmt)
        {
            std::cerr << "Expression statement test failed!\n";
            return 1;
        }

        auto *call = dynamic_cast<CallExpr *>(exprStmt->expression.get());

        if (!call || call->arguments.size() != 1)
        {
            std::cerr << "Statement call test failed!\n";
            return 1;
        }

        auto *callee = dynamic_cast<IdentifierExpr *>(call->callee.get());

        if (!callee || callee->name != "print")
        {
            std::cerr << "Statement callee test failed!\n";
            return 1;
        }
    }

    std::cout << "All parser tests passed!\n";
    return 0;
}
