#include "parser.h"

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

namespace parser {
Parser::Parser() {}

std::unique_ptr<ast::AstNode> Parser::parse(lexer::Lexer &input) const {
  tokens::Token next = input.peek();
  auto ret = std::visit(
      overloaded{[&](const tokens::Def &) {
                   input.pop();
                   return parseDefinition(input);
                 },
                 [&](const tokens::Extern &) {
                   input.pop();
                   return parseExtern(input);
                 },
                 [&](const auto &) { return parseTopLevelExpr(input); }},
      next);
  try {
    auto end = std::get<tokens::Character>(input.pop());
    if (end.character != ';') {
      throw std::runtime_error("no end semicolon");
    }
  } catch (const std::bad_variant_access &e) {
    throw std::runtime_error("invalid end character");
  }
  return ret;
}

std::unique_ptr<ast::expr::ExprNode>
Parser::parseExpression(lexer::Lexer &input) const {
  auto token = input.pop();
  auto lhs = parsePrimary(token, input);
  if (!lhs) {
    return nullptr;
  }

  if (getOpPrecedence(input.peek()) != -1) {
    token = input.pop();
    return parseBinOpRhs(0, std::move(lhs), token, input);
  }
  return lhs;
}

std::unique_ptr<ast::Prototype>
Parser::parsePrototype(lexer::Lexer &input) const {
  auto next = input.pop();
  std::string fnName;
  try {
    fnName = std::get<tokens::Identifier>(next).ident;
  } catch (const std::bad_variant_access &) {
    throw std::runtime_error("Expected function name in prototype");
  }

  assertIsCharacter(input.pop(), '(', "prototype must open with '('");

  std::vector<std::string> argNames;
  tokens::Token token;
  while (std::holds_alternative<tokens::Identifier>(token = input.pop())) {
    argNames.push_back(std::get<tokens::Identifier>(token).ident);
    token = input.pop();
    wchar_t character;
    try {
      character = std::get<tokens::Character>(token).character;
    } catch (const std::bad_variant_access &) {
      throw std::runtime_error(
          "prototype arguments must be split by , and ended with )");
    }

    if (character == ')') {
      break;
    }

    if (character != ',') {
      throw std::runtime_error("prototype arguments must be split by ,");
    }
  }

  assertIsCharacter(token, ')', "prototype must close with ')'");

  return std::make_unique<ast::Prototype>(fnName, std::move(argNames));
}

std::unique_ptr<ast::expr::ExprNode>
Parser::parsePrimary(const tokens::Token &token, lexer::Lexer &input) const {
  return std::visit(
      overloaded{
          [&](const tokens::Identifier &ident) {
            return parseIdentifier(ident, input);
          },
          [&](const tokens::Number &number) { return parseNumber(number); },
          [&](const tokens::Character &character) { return parseParen(input); },
          [](const auto &) {
            return std::unique_ptr<ast::expr::ExprNode>(nullptr);
          }},
      token);
}

std::unique_ptr<ast::AstNode>
Parser::parseTopLevelExpr(lexer::Lexer &input) const {
  if (auto E = parseExpression(input)) {
    auto proto = std::make_unique<ast::Prototype>("__anon_expr",
                                                  std::vector<std::string>{});
    return std::make_unique<ast::AstNode>(
        ast::Function(std::move(proto), std::move(E)));
  }
  return nullptr;
}

std::unique_ptr<ast::expr::ExprNode>
Parser::parseParen(lexer::Lexer &input) const {
  auto v = parseExpression(input);
  if (!v)
    return nullptr;

  auto top = input.peek();
  if (auto token = std::get_if<tokens::Character>(&top);
      token && token->character != ')') {
    throw std::runtime_error("unclosed parentheses!");
  }
  input.pop();
  return v;
}

std::unique_ptr<ast::expr::ExprNode>
Parser::parseIdentifier(const tokens::Identifier &ident,
                        lexer::Lexer &input) const {
  std::string idName = ident.ident;

  if (!(input.peek() == tokens::Token{tokens::Character{L'('}})) {
    return std::make_unique<ast::expr::ExprNode>(idName);
  }

  assertIsCharacter(input.pop(), '(', "prototype must open with '('");

  std::vector<std::unique_ptr<ast::expr::ExprNode>> args;
  tokens::Token token;
  while (true) {
    if (auto arg = parseExpression(input))
      args.push_back(std::move(arg));
    else
      return nullptr;
    token = input.pop();
    wchar_t character;
    try {
      character = std::get<tokens::Character>(token).character;
    } catch (const std::bad_variant_access &) {
      throw std::runtime_error(
          "call arguments must be split by , and ended with )");
    }

    if (character == ')') {
      break;
    }

    if (character != ',') {
      throw std::runtime_error("call arguments must be split by ,");
    }
  }

  assertIsCharacter(token, ')', "call must close with ')'");

  return std::make_unique<ast::expr::ExprNode>(
      ast::expr::Call(idName, std::move(args)));
}

std::unique_ptr<ast::expr::ExprNode>
Parser::parseNumber(const tokens::Number &number) const {
  auto expr = std::make_unique<ast::expr::ExprNode>(number.val);
  return std::move(expr);
}

std::unique_ptr<ast::expr::ExprNode>
Parser::parseBinOpRhs(int exprPrec, std::unique_ptr<ast::expr::ExprNode> lhs,
                      tokens::Token &op, lexer::Lexer &input) const {
  while (true) {
    int tokPrec = getOpPrecedence(op);

    if (tokPrec < exprPrec) {
      return lhs;
    }

    wchar_t binOp;
    try {
      binOp = std::get<tokens::Character>(op).character;
    } catch (const std::bad_variant_access &) {
      throw std::runtime_error("binary operator must be a character!");
    }

    auto token = input.pop();
    auto rhs = parsePrimary(token, input);
    if (!rhs) {
      return nullptr;
    }

    op = input.peek();
    int nextPrec = getOpPrecedence(op);
    if (nextPrec != -1)
      input.pop();
    if (tokPrec < nextPrec) {
      rhs = parseBinOpRhs(tokPrec + 1, std::move(rhs), op, input);
      if (!rhs) {
        return nullptr;
      }
    }

    lhs = std::make_unique<ast::expr::ExprNode>(
        ast::expr::Binary(binOp, std::move(lhs), std::move(rhs)));
  }
}

int Parser::getOpPrecedence(tokens::Token token) const {
  if (!std::holds_alternative<tokens::Character>(token)) {
    return -1;
  }

  char character = std::get<tokens::Character>(token).character;

  auto search = binOpPrecedence.find(character);
  if (search == binOpPrecedence.end())
    return -1;
  return search->second;
}

std::unique_ptr<ast::AstNode> Parser::parseExtern(lexer::Lexer &input) const {
  auto proto = *parsePrototype(input);
  proto.isExtern = true;
  return std::make_unique<ast::AstNode>(proto);
}

std::unique_ptr<ast::AstNode>
Parser::parseDefinition(lexer::Lexer &input) const {
  auto proto = parsePrototype(input);
  if (!proto)
    return nullptr;

  if (auto E = parseExpression(input))
    return std::make_unique<ast::AstNode>(
        ast::Function(std::move(proto), std::move(E)));
  return nullptr;
}

void Parser::assertIsCharacter(const tokens::Token &tkn, char target,
                               const std::string &error) const {
  using namespace std::string_literals;
  try {
    wchar_t c = std::get<tokens::Character>(tkn).character;
    if (c != target) {
      throw std::runtime_error(error);
    }
  } catch (const std::bad_variant_access &) {
    throw std::runtime_error("Assert attempted without character!");
  }
}
} // namespace parser
