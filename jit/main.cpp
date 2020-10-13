#include <iostream>
#include <string>
#include <sstream>

#include "parser.h"

int main()
{
	std::wstring source{ L"def plustwo(x) x + 2;" };
	std::wstringstream sourceStream{ source };
	
	lexer::Lexer lexer{ sourceStream };

	parser::Parser parser{};
	auto ast = parser.parse(lexer);

	return 0;
}
