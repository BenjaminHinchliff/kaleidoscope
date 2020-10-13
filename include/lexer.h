#ifndef LEXER_H_
#define LEXER_H_

#include <iostream>
#include <cctype>
#include <string>
#include <list>

#include "tokens.h"

namespace lexer
{
    class Lexer
    {
    public:
        Lexer(std::wistream& input);

        tokens::Token peek(size_t forward = 0ui64) const;
        tokens::Token pop();

    private:
        std::list<tokens::Token> tokens;

        tokens::Token extractToken(std::wistream& input);
    };
}

#endif // !LEXER_H_
