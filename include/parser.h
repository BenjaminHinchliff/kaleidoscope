#ifndef PARSER_H_
#define PARSER_H_

#include <memory>
#include <unordered_map>

#include "ast.h"
#include "lexer.h"
#include "tokens.h"

namespace parser {
class Parser {
public:
  Parser();

  std::unique_ptr<ast::AstNode> parse(lexer::Lexer &input);
  std::unique_ptr<ast::expr::ExprNode> parseExpression(lexer::Lexer &input);
  std::unique_ptr<ast::Prototype> parsePrototype(lexer::Lexer &input);
  std::unique_ptr<ast::expr::ExprNode> parsePrimary(const tokens::Token &token,
                                                    lexer::Lexer &input);

private:
  std::unique_ptr<ast::AstNode> parseTopLevelExpr(lexer::Lexer &input);
  std::unique_ptr<ast::expr::ExprNode> parseParen(lexer::Lexer &input);
  std::unique_ptr<ast::expr::ExprNode>
  parseIdentifier(const tokens::Identifier &ident, lexer::Lexer &input);
  std::unique_ptr<ast::expr::ExprNode>
  parseNumber(const tokens::Number &number);
  std::unique_ptr<ast::expr::ExprNode>
  parseBinOpRhs(int exprPrec, std::unique_ptr<ast::expr::ExprNode> lhs,
                tokens::Token &op, lexer::Lexer &input);
  int getOpPrecedence(tokens::Token token);
  std::unique_ptr<ast::AstNode> parseExtern(lexer::Lexer &input);
  std::unique_ptr<ast::AstNode> parseDefinition(lexer::Lexer &input);
  void assertIsCharacter(const tokens::Token &tkn, wchar_t target,
                         const std::string &error);

public:
  std::unordered_map<wchar_t, int> binOpPrecedence{
      {'<', 10}, {'+', 20}, {'-', 20}, {'*', 40}};
};
} // namespace parser

#endif // !PARSER_H_
