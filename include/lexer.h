#ifndef LEXER_H_
#define LEXER_H_

#include <list>
#include <cctype>
#include <string>

#include "tokens.h"

namespace lexer {
class Lexer {
public:
  Lexer(std::istream &input);

  tokens::Token peek(size_t forward = 0ull) const;
  tokens::Token pop();

private:
  std::list<tokens::Token> tokens;

  tokens::Token extractToken(std::istream &input);
};
} // namespace lexer

#endif // !LEXER_H_
