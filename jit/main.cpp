#include <iostream>
#include <string>
#include <sstream>

#include "parser.h"

int main()
{
	std::string source{ "def plustwo(x) x + 2; plustwo(2);" };
	std::stringstream sourceStream{ source };
	
	lexer::Lexer lexer{ sourceStream };

	llvm::LLVMContext context;
	llvm::IRBuilder<> builder(context);
	auto llvmModule = std::make_shared<llvm::Module>("KaleidoscopeJIT", context);
	ast::named_values_t namedValues;

	parser::Parser parser{};

	while (!std::holds_alternative<tokens::Eof>(lexer.peek()))
	{
		auto ast = parser.parse(lexer);
		auto fnIR = std::visit([&](const auto& ast) { return ast.codegen(context, builder, llvmModule, namedValues); }, *ast);
		auto end = std::get<tokens::Character>(lexer.pop());
		if (end.character != ';')
		{
			std::cerr << "each expression must end with semicolon!";
			return 1;
		}
		std::cout << "Partial:\n";
		fnIR->print(llvm::outs(), nullptr);
	}

	std::cout << "Final:\n";
	llvmModule->print(llvm::outs(), nullptr);

	return 0;
}
