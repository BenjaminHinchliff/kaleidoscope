#ifndef AST_H_
#define AST_H_

#include <iostream>
#include <variant>
#include <vector>

namespace ast {
class AstInterface {};

namespace expr {
class Number;
class Variable;
class Binary;
class Call;

using ExprNode = std::variant<Number, Variable, Binary, Call>;

class ExprInterface : public AstInterface {};

class Number : public ExprInterface {
public:
  Number(double val);

  friend std::wostream &operator<<(std::wostream &out, const Number &expr);

  double val;
};

class Variable : public ExprInterface {
public:
  Variable(const std::wstring &name);

  std::wstring name;

  friend std::wostream &operator<<(std::wostream &out, const Variable &var);
};

class Binary : public ExprInterface {
public:
  Binary::Binary(wchar_t op, std::unique_ptr<ExprNode> lhs,
                 std::unique_ptr<ExprNode> rhs);

  wchar_t op;
  std::unique_ptr<ExprNode> lhs;
  std::unique_ptr<ExprNode> rhs;

  friend std::wostream &operator<<(std::wostream &out, const Binary &op);
};

class Call : public ExprInterface {
public:
  Call(const std::wstring &callee, std::vector<std::unique_ptr<ExprNode>> args);

  std::wstring callee;
  std::vector<std::unique_ptr<ExprNode>> args;

  friend std::wostream &operator<<(std::wostream &out, const Call &call);
};
} // namespace expr

class Prototype : public AstInterface {
public:
  Prototype(const std::wstring &name, std::vector<std::wstring> args);

  std::wstring name;
  std::vector<std::wstring> args;
};

class Function : public AstInterface {
public:
  Function(std::unique_ptr<Prototype> proto,
           std::unique_ptr<expr::ExprNode> body);

  std::unique_ptr<Prototype> proto;
  std::unique_ptr<expr::ExprNode> body;
};

using AstNode = std::variant<Prototype, Function>;
} // namespace ast

#endif // !AST_H_
