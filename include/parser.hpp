// I wish this could be simplified...

#include <memory>

#include "ast.hpp"
#include "lexer.hpp"
#include "token.hpp"

// Creates AST from tokens.
class Parser
{
  public:
    explicit Parser(Lexer &lexer);

    IntegerExpr parseInteger();

    // Parse an arthimetic expr.
    std::unique_ptr<Expr> parseArth();
    // Parse a statement.
    std::unique_ptr<Stmt> parseStatement();

  private:
    // Lexer for this parser.
    Lexer &lexer_;
    Token current_;

    // Move to the next token.
    void advance();

    std::unique_ptr<Expr> parseCall();
    std::unique_ptr<Expr> parseMult();
    std::unique_ptr<Expr> parsePrimary();
    std::unique_ptr<Expr> parseUnary();
};
