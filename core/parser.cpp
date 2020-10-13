#include "parser.h"

namespace parser
{
	Parser::Parser()
	{
	}

	std::unique_ptr<ast::AstNode> Parser::parse(lexer::Lexer& input)
	{
		tokens::Token next = input.pop();
		return std::visit(
			overloaded
			{
				[&](const tokens::Def&) { return parseDefinition(input); },
				[&](const tokens::Extern&) { return parseExtern(input); },
				[&](const auto&) { return parseTopLevelExpr(input); }
			},
			next
		);
	}

	std::unique_ptr<ast::expr::ExprNode> Parser::parseExpression(lexer::Lexer& input)
	{
		auto token = input.pop();
		auto lhs = parsePrimary(token, input);
		if (!lhs)
		{
			return nullptr;
		}

		if (getOpPrecedence(input.peek()) != -1)
		{
			token = input.pop();
			return parseBinOpRhs(0, std::move(lhs), token, input);
		}
		return lhs;
	}

	std::unique_ptr<ast::Prototype> Parser::parsePrototype(lexer::Lexer& input)
	{
		auto next = input.pop();
		std::wstring fnName;
		try
		{
			fnName = std::get<tokens::Identifier>(next).ident;
		}
		catch (const std::bad_variant_access&)
		{
			throw std::runtime_error("Expected function name in prototype");
		}

		assertIsCharacter(input.pop(), '(', "prototype must open with '('");

		std::vector<std::wstring> argNames;
		tokens::Token token;
		while (std::holds_alternative<tokens::Identifier>(token = input.pop()))
		{
			argNames.push_back(std::get<tokens::Identifier>(token).ident);
			token = input.pop();
			wchar_t character;
			try
			{
				character = std::get<tokens::Character>(token).character;
			}
			catch (const std::bad_variant_access&)
			{
				throw std::runtime_error("prototype arguments must be split by , and ended with )");
			}

			if (character == ')')
			{
				break;
			}

			if (character != ',')
			{
				throw std::runtime_error("prototype arguments must be split by ,");
			}
		}

		assertIsCharacter(token, ')', "prototype must close with ')'");

		return std::make_unique<ast::Prototype>(fnName, std::move(argNames));
	}

	std::unique_ptr<ast::expr::ExprNode> Parser::parsePrimary(const tokens::Token& token, lexer::Lexer& input)
	{
		return std::visit(
			overloaded
			{
				[&](const tokens::Identifier& ident) { return parseIdentifier(ident, input); },
				[&](const tokens::Number& number) { return parseNumber(number); },
				[&](const tokens::Character& character) { return parseParen(input); },
				[](const auto&) { return std::unique_ptr<ast::expr::ExprNode>(nullptr); }
			},
			token
		);
	}

	std::unique_ptr<ast::AstNode> Parser::parseTopLevelExpr(lexer::Lexer& input)
	{
		if (auto E = parseExpression(input))
		{
			auto proto = std::make_unique<ast::Prototype>(L"__anon_expr", std::vector<std::wstring>{});
			return std::make_unique<ast::AstNode>(ast::Function(std::move(proto), std::move(E)));
		}
		return nullptr;
	}

	std::unique_ptr<ast::expr::ExprNode> Parser::parseParen(lexer::Lexer& input)
	{
		auto v = parseExpression(input);
		if (!v)
			return nullptr;

		if (auto token = std::get_if<tokens::Character>(&input.peek()); token && token->character != ')')
		{
			throw std::runtime_error("unclosed parentheses!");
		}
		input.pop();
		return v;
	}

	std::unique_ptr<ast::expr::ExprNode> Parser::parseIdentifier(const tokens::Identifier& ident, lexer::Lexer& input)
	{
		std::wstring idName = ident.ident;

		if (!(input.peek() == tokens::Token{ tokens::Character{ L'(' } }))
		{
			return std::make_unique<ast::expr::ExprNode>(idName);
		}

		assertIsCharacter(input.pop(), '(', "prototype must open with '('");

		std::vector<std::unique_ptr<ast::expr::ExprNode>> args;
		tokens::Token token;
		while (true)
		{
			if (auto arg = parseExpression(input))
				args.push_back(std::move(arg));
			else
				return nullptr;
			token = input.pop();
			wchar_t character;
			try
			{
				character = std::get<tokens::Character>(token).character;
			}
			catch (const std::bad_variant_access&)
			{
				throw std::runtime_error("call arguments must be split by , and ended with )");
			}

			if (character == ')')
			{
				break;
			}

			if (character != ',')
			{
				throw std::runtime_error("call arguments must be split by ,");
			}
		}

		assertIsCharacter(token, ')', "call must close with ')'");

		return std::make_unique<ast::expr::ExprNode>(ast::expr::Call(idName, std::move(args)));
	}

	std::unique_ptr<ast::expr::ExprNode> Parser::parseNumber(const tokens::Number& number)
	{
		auto expr = std::make_unique<ast::expr::ExprNode>(number.val);
		return std::move(expr);
	}

	std::unique_ptr<ast::expr::ExprNode> Parser::parseBinOpRhs(
		int exprPrec,
		std::unique_ptr<ast::expr::ExprNode> lhs,
		tokens::Token& op,
		lexer::Lexer& input
	)
	{
		while (true)
		{
			int tokPrec = getOpPrecedence(op);

			if (tokPrec < exprPrec)
			{
				return lhs;
			}

			wchar_t binOp;
			try
			{
				binOp = std::get<tokens::Character>(op).character;
			}
			catch (const std::bad_variant_access&)
			{
				throw std::runtime_error("binary operator must be a character!");
			}

			auto token = input.pop();
			auto rhs = parsePrimary(token, input);
			if (!rhs)
			{
				return nullptr;
			}

			op = input.peek();
			int nextPrec = getOpPrecedence(op);
			if (nextPrec != -1) input.pop();
			if (tokPrec < nextPrec)
			{
				rhs = parseBinOpRhs(tokPrec + 1, std::move(rhs), op, input);
				if (!rhs)
				{
					return nullptr;
				}
			}

			lhs = std::make_unique<ast::expr::ExprNode>(ast::expr::Binary(binOp, std::move(lhs), std::move(rhs)));
		}
	}

	int Parser::getOpPrecedence(tokens::Token token)
	{
		if (!std::holds_alternative<tokens::Character>(token))
		{
			return -1;
		}

		wchar_t character = std::get<tokens::Character>(token).character;

		auto search = binOpPrecedence.find(character);
		if (search == binOpPrecedence.end())
			return -1;
		return search->second;
	}

	std::unique_ptr<ast::AstNode> Parser::parseExtern(lexer::Lexer& input)
	{
		auto proto = *parsePrototype(input);
		return std::make_unique<ast::AstNode>(proto);
	}

	std::unique_ptr<ast::AstNode> Parser::parseDefinition(lexer::Lexer& input)
	{
		auto proto = parsePrototype(input);
		if (!proto)
			return nullptr;

		if (auto E = parseExpression(input))
			return std::make_unique<ast::AstNode>(ast::Function(std::move(proto), std::move(E)));
		return nullptr;
	}

	void Parser::assertIsCharacter(const tokens::Token& tkn, wchar_t target, const std::string& error)
	{
		using namespace std::string_literals;
		try
		{
			wchar_t c = std::get<tokens::Character>(tkn).character;
			if (c != target)
			{
				throw std::runtime_error(error);
			}
		}
		catch (const std::bad_variant_access&)
		{
			throw std::runtime_error("Assert attempted without character!");
		}
	}
}
