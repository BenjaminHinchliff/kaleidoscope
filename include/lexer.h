#ifndef LEXER_H_
#define LEXER_H_

#include <cctype>
#include <list>
#include <string>
#include <limits>

#include "tokens.h"

namespace lexer {
class Lexer {
public:
  Lexer(std::istream &input);

  tokens::Token peek(size_t forward = 0ull) const;
  tokens::Token pop();
  bool empty() const noexcept;

private:
  std::list<tokens::Token> tokens;

  tokens::Token extractToken(std::istream &input);
};
} // namespace lexer

#endif // !LEXER_H_
