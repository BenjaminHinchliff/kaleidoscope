#include <gtest/gtest.h>

#include "lexer.h"

TEST(Lexer, PeekWorks) {
  std::wstring testInput{L"12 34"};
  std::wstringstream ss;
  ss << testInput;
  lexer::Lexer lexer(ss);
  ASSERT_EQ(lexer.peek(), tokens::Token(tokens::Number(12)));
  ASSERT_EQ(lexer.peek(1), tokens::Token(tokens::Number(34)));
}

TEST(Lexer, PopWorks) {
  std::wstring testInput{L"12 34"};
  std::wstringstream ss;
  ss << testInput;
  lexer::Lexer lexer(ss);
  ASSERT_EQ(lexer.pop(), tokens::Token(tokens::Number(12)));
  ASSERT_EQ(lexer.pop(), tokens::Token(tokens::Number(34)));
}

TEST(Lexer, ParsesAllIdentifiersCorrectly) {
  std::wstring testInput{L"def extern x 12.34 ("};
  std::wstringstream ss;
  ss << testInput;
  lexer::Lexer lexer(ss);

  ASSERT_TRUE(std::holds_alternative<tokens::Def>(lexer.pop()));
  ASSERT_TRUE(std::holds_alternative<tokens::Extern>(lexer.pop()));
  ASSERT_EQ(lexer.pop(), tokens::Token(tokens::Identifier(L"x")));
  ASSERT_EQ(lexer.pop(), tokens::Token(tokens::Number(12.34)));
  ASSERT_EQ(lexer.pop(), tokens::Token(tokens::Character(L'(')));
}
