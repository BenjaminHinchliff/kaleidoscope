#include <gtest/gtest.h>

#include <vector>

#include "lexer.h"
#include "parser.h"

TEST(Parser, BinOpParsingWorks)
{
	std::wstring input{ L"1 + 2 * 3 - 4" };
	std::wstringstream ss;
	ss << input;
	lexer::Lexer lexer(ss);

	parser::Parser parser;
	auto binOp = parser.parseExpression(lexer);

	const auto& op = std::get<ast::expr::Binary>(*binOp);
	ASSERT_EQ(std::get<ast::expr::Number>(*op.rhs).val, 4);
	const auto& op2 = std::get<ast::expr::Binary>(*op.lhs);
	ASSERT_EQ(std::get<ast::expr::Number>(*op2.lhs).val, 1);
	const auto& op3 = std::get<ast::expr::Binary>(*op2.rhs);
	ASSERT_EQ(std::get<ast::expr::Number>(*op3.lhs).val, 2);
	ASSERT_EQ(std::get<ast::expr::Number>(*op3.rhs).val, 3);
}

TEST(Parser, PrototypeParsingWorks)
{
	std::wstring input{ L"proto(x, y, z, j)" };
	std::wstringstream ss;
	ss << input;
	lexer::Lexer lexer(ss);

	parser::Parser parser;
	auto proto = parser.parsePrototype(lexer);

	ASSERT_EQ(proto->name, L"proto");
	std::vector<std::wstring> args{ L"x", L"y", L"z", L"j" };
	for (size_t i = 0; i < args.size(); ++i)
	{
		ASSERT_EQ(proto->args[i], args[i]);
	}
}

TEST(Parser, CallParsingWorks)
{
	std::wstring input{ L"call(1, 2, 3, 4)" };
	std::wstringstream ss;
	ss << input;
	lexer::Lexer lexer(ss);

	parser::Parser parser;
	auto call = std::move(std::get<ast::expr::Call>(*parser.parsePrimary(lexer.pop(), lexer)));
	
	ASSERT_EQ(call.callee, L"call");
	std::vector<double> args{ 1.0, 2.0, 3.0, 4.0 };
	for (size_t i = 0; i < args.size(); ++i)
	{
		ASSERT_EQ(std::get<ast::expr::Number>(*call.args[i]).val, args[i]);
	}
}

TEST(Parser, VariableParsingWorks)
{
	std::wstring input{ L"x" };
	std::wstringstream ss;
	ss << input;
	lexer::Lexer lexer(ss);

	parser::Parser parser;
	auto call = std::move(std::get<ast::expr::Variable>(*parser.parsePrimary(lexer.pop(), lexer)));
	ASSERT_EQ(call.name, L"x");
}

TEST(Parser, ParentheticalParsingWorks)
{
	std::wstring input{ L"1 * (2 - 3)" };
	std::wstringstream ss;
	ss << input;
	lexer::Lexer lexer(ss);

	parser::Parser parser;
	auto ops = std::move(std::get<ast::expr::Binary>(*parser.parseExpression(lexer)));
	ASSERT_EQ(std::get<ast::expr::Number>(*ops.lhs).val, 1);
	ASSERT_EQ(ops.op, L'*');
	const auto& rhs = std::get<ast::expr::Binary>(*ops.rhs);
	ASSERT_EQ(std::get<ast::expr::Number>(*rhs.lhs).val, 2);
	ASSERT_EQ(rhs.op, L'-');
	ASSERT_EQ(std::get<ast::expr::Number>(*rhs.rhs).val, 3);
}

TEST(Parser, DefinitionParsingWorks)
{
	std::wstring input{ L"def plustwo(x) x + 2;" };
	std::wstringstream ss;
	ss << input;
	lexer::Lexer lexer(ss);

	parser::Parser parser;
	auto function = std::move(std::get<ast::Function>(*parser.parse(lexer)));
	ASSERT_EQ(function.proto->name, L"plustwo");
	ASSERT_EQ(function.proto->args[0], L"x");
	const auto& body = std::get<ast::expr::Binary>(*function.body);
	ASSERT_EQ(std::get<ast::expr::Variable>(*body.lhs).name, L"x");
	ASSERT_EQ(body.op, L'+');
	ASSERT_EQ(std::get<ast::expr::Number>(*body.rhs).val, 2);
}

TEST(Parser, ExternParsingWorks)
{
	std::wstring input{ L"extern sin(x);" };
	std::wstringstream ss;
	ss << input;
	lexer::Lexer lexer(ss);

	parser::Parser parser;
	auto function = std::move(std::get<ast::Prototype>(*parser.parse(lexer)));
	ASSERT_EQ(function.name, L"sin");
	ASSERT_EQ(function.args[0], L"x");
}
