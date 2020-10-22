#include "tokens.hpp"

namespace tokens {
bool Eof::operator==(const Eof &other) const noexcept { return true; }

std::ostream &operator<<(std::ostream &out, const Eof &c) {
  out << "Eof";
  return out;
}

bool Def::operator==(const Def &other) const noexcept { return true; }

std::ostream &operator<<(std::ostream &out, const Def &c) {
  out << "Def";
  return out;
}

bool Extern::operator==(const Extern &other) const noexcept { return true; }

std::ostream &operator<<(std::ostream &out, const Extern &c) {
  out << "Extern";
  return out;
}

Identifier::Identifier(const std::string &ident) : ident(ident) {}

bool Identifier::operator==(const Identifier &other) const noexcept {
  return ident == other.ident;
}

std::ostream &operator<<(std::ostream &out, const Identifier &c) {
  out << "Identifier(\"" << c.ident << "\")";
  return out;
}

Number::Number(double val) : val(val) {}

bool Number::operator==(const Number &other) const noexcept {
  return val == other.val;
}

std::ostream &operator<<(std::ostream &out, const Number &c) {
  out << "Number(" << c.val << ")";
  return out;
}

Character::Character(char character) : character(character) {}

bool Character::operator==(const Character &other) const noexcept {
  return character == other.character;
}

std::ostream &operator<<(std::ostream &out, const Character &c) {
  out << "Character(\'" << c.character << "\')";
  return out;
}
} // namespace tokens
