#include <gtest/gtest.h>

#include <vector>

#include "lexer.hpp"
#include "parser.hpp"

TEST(Parser, BinOpParsingWorks) {
  std::string input{"1 + 2 * 3 - 4"};
  std::stringstream ss;
  ss << input;
  lexer::Lexer lexer(ss);

  parser::Parser parser;
  auto binOp = parser.parseExpression(lexer);

  const auto &op = std::get<ast::expr::Binary>(*binOp);
  ASSERT_EQ(std::get<ast::expr::Number>(*op.rhs).val, 4);
  const auto &op2 = std::get<ast::expr::Binary>(*op.lhs);
  ASSERT_EQ(std::get<ast::expr::Number>(*op2.lhs).val, 1);
  const auto &op3 = std::get<ast::expr::Binary>(*op2.rhs);
  ASSERT_EQ(std::get<ast::expr::Number>(*op3.lhs).val, 2);
  ASSERT_EQ(std::get<ast::expr::Number>(*op3.rhs).val, 3);
}

TEST(Parser, PrototypeParsingWorks) {
  std::string input{"proto(x, y, z, j)"};
  std::stringstream ss;
  ss << input;
  lexer::Lexer lexer(ss);

  parser::Parser parser;
  auto proto = parser.parsePrototype(lexer);

  ASSERT_EQ(proto->name, "proto");
  std::vector<std::string> args{"x", "y", "z", "j"};
  for (size_t i = 0; i < args.size(); ++i) {
    ASSERT_EQ(proto->args[i], args[i]);
  }
}

TEST(Parser, CallParsingWorks) {
  std::string input{"call(1, 2, 3, 4)"};
  std::stringstream ss;
  ss << input;
  lexer::Lexer lexer(ss);

  parser::Parser parser;
  auto call = std::move(
      std::get<ast::expr::Call>(*parser.parsePrimary(lexer.pop(), lexer)));

  ASSERT_EQ(call.callee, "call");
  std::vector<double> args{1.0, 2.0, 3.0, 4.0};
  for (size_t i = 0; i < args.size(); ++i) {
    ASSERT_EQ(std::get<ast::expr::Number>(*call.args[i]).val, args[i]);
  }
}

TEST(Parser, VariableParsingWorks) {
  std::string input{"x"};
  std::stringstream ss;
  ss << input;
  lexer::Lexer lexer(ss);

  parser::Parser parser;
  auto call = std::move(
      std::get<ast::expr::Variable>(*parser.parsePrimary(lexer.pop(), lexer)));
  ASSERT_EQ(call.name, "x");
}

TEST(Parser, ParentheticalParsingWorks) {
  std::string input{"1 * (2 - 3)"};
  std::stringstream ss;
  ss << input;
  lexer::Lexer lexer(ss);

  parser::Parser parser;
  auto ops =
      std::move(std::get<ast::expr::Binary>(*parser.parseExpression(lexer)));
  ASSERT_EQ(std::get<ast::expr::Number>(*ops.lhs).val, 1);
  ASSERT_EQ(ops.op, '*');
  const auto &rhs = std::get<ast::expr::Binary>(*ops.rhs);
  ASSERT_EQ(std::get<ast::expr::Number>(*rhs.lhs).val, 2);
  ASSERT_EQ(rhs.op, '-');
  ASSERT_EQ(std::get<ast::expr::Number>(*rhs.rhs).val, 3);
}

TEST(Parser, DefinitionParsingWorks) {
  std::string input{"def plustwo(x) x + 2;"};
  std::stringstream ss;
  ss << input;
  lexer::Lexer lexer(ss);

  parser::Parser parser;
  auto function = std::move(std::get<ast::Function>(*parser.parse(lexer)));
  ASSERT_EQ(function.proto->name, "plustwo");
  ASSERT_EQ(function.proto->args[0], "x");
  const auto &body = std::get<ast::expr::Binary>(*function.body);
  ASSERT_EQ(std::get<ast::expr::Variable>(*body.lhs).name, "x");
  ASSERT_EQ(body.op, '+');
  ASSERT_EQ(std::get<ast::expr::Number>(*body.rhs).val, 2);
}

TEST(Parser, ExternParsingWorks) {
  std::string input{"extern sin(x);"};
  std::stringstream ss;
  ss << input;
  lexer::Lexer lexer(ss);

  parser::Parser parser;
  auto function = std::move(std::get<ast::Prototype>(*parser.parse(lexer)));
  ASSERT_EQ(function.name, "sin");
  ASSERT_EQ(function.args[0], "x");
}
