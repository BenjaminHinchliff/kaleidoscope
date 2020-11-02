#ifndef PARSER_PARSER_HPP_
#define PARSER_PARSER_HPP_

#include <memory>
#include <unordered_map>

#include "ast.hpp"
#include "lexer.hpp"
#include "tokens.hpp"

namespace parser {
class Parser {
public:
  Parser();

  std::unique_ptr<ast::AstNode> parse(lexer::Lexer &input) const;
  std::unique_ptr<ast::expr::ExprNode>
  parseExpression(lexer::Lexer &input) const;
  std::unique_ptr<ast::Prototype> parsePrototype(lexer::Lexer &input) const;
  std::unique_ptr<ast::expr::ExprNode> parsePrimary(const tokens::Token &token,
                                                    lexer::Lexer &input) const;

private:
  std::unique_ptr<ast::AstNode> parseTopLevelExpr(lexer::Lexer &input) const;
  std::unique_ptr<ast::expr::ExprNode> parseParen(lexer::Lexer &input) const;
  std::unique_ptr<ast::expr::ExprNode>
  parseIdentifier(const tokens::Identifier &ident, lexer::Lexer &input) const;
  std::unique_ptr<ast::expr::ExprNode>
  parseNumber(const tokens::Number &number) const;
  std::unique_ptr<ast::expr::ExprNode>
  parseBinOpRhs(int exprPrec, std::unique_ptr<ast::expr::ExprNode> lhs,
                tokens::Token &op, lexer::Lexer &input) const;
  int getOpPrecedence(tokens::Token token) const;
  std::unique_ptr<ast::AstNode> parseExtern(lexer::Lexer &input) const;
  std::unique_ptr<ast::AstNode> parseDefinition(lexer::Lexer &input) const;
  void assertIsCharacter(const tokens::Token &tkn, char target,
                         const std::string &error) const;

public:
  std::unordered_map<char, int> binOpPrecedence{
      {'<', 10}, {'+', 20}, {'-', 20}, {'*', 40}, {'/', 20}};
};
} // namespace parser

#endif // !PARSER_PARSER_HPP_
