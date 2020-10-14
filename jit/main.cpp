#include <iostream>
#include <string>
#include <sstream>

#include "parser.h"

int main()
{
	std::string source{ "def plustwo(x) x + 2;" };
	std::stringstream sourceStream{ source };
	
	lexer::Lexer lexer{ sourceStream };

	parser::Parser parser{};
	auto ast = parser.parse(lexer);

	return 0;
}
