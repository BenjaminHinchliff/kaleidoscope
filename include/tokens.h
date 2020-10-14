#ifndef TOKENS_H_
#define TOKENS_H_

#include <iostream>
#include <variant>

namespace tokens {
class Eof {
public:
  bool operator==(const Eof &other) const noexcept;

  friend std::ostream &operator<<(std::ostream &out, const Eof &c);
};

class Def {
public:
  bool operator==(const Def &other) const noexcept;

  friend std::ostream &operator<<(std::ostream &out, const Def &c);
};

class Extern {
public:
  bool operator==(const Extern &other) const noexcept;

  friend std::ostream &operator<<(std::ostream &out, const Extern &c);
};

class Identifier {
public:
  Identifier(const std::string &ident);

  bool operator==(const Identifier &other) const noexcept;

  friend std::ostream &operator<<(std::ostream &out, const Identifier &c);

public:
  std::string ident;
};

class Number {
public:
  Number(double val);

  bool operator==(const Number &other) const noexcept;

  friend std::ostream &operator<<(std::ostream &out, const Number &c);

public:
  double val;
};

class Character {
public:
  Character(char character);

  bool operator==(const Character &other) const noexcept;

  friend std::ostream &operator<<(std::ostream &out, const Character &c);

public:
  char character;
};

using Token = std::variant<Def, Extern, Identifier, Number, Character, Eof>;
} // namespace tokens

#endif // !TOKENS_H_
