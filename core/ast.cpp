#include "ast.h"

namespace ast {
namespace expr {
Number::Number(double val) : val(val) {}

std::wostream &operator<<(std::wostream &out, const Number &expr) {
  out << "Number{" << expr.val << '}';
  return out;
}

Variable::Variable(const std::wstring &name) : name(name) {}

std::wostream &operator<<(std::wostream &out, const Variable &var) {
  out << "Variable{" << var.name << "}";
  return out;
}

Binary::Binary(wchar_t op, std::unique_ptr<ExprNode> lhs,
               std::unique_ptr<ExprNode> rhs)
    : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

std::wostream &operator<<(std::wostream &out, const Binary &op) {
  out << "Binary{"
      << "lhs: ";
  std::visit([&](const auto &tkn) { out << tkn; }, *op.lhs);
  out << ", op: " << op.op << ", rhs: ";
  std::visit([&](const auto &tkn) { out << tkn; }, *op.rhs);
  out << '}';
  return out;
}

Call::Call(const std::wstring &callee,
           std::vector<std::unique_ptr<ExprNode>> args)
    : callee(callee), args(std::move(args)) {}

std::wostream &operator<<(std::wostream &out, const Call &call) {
  out << "Call{TBD}";
  return out;
}
} // namespace expr

Prototype::Prototype(const std::wstring &name, std::vector<std::wstring> args)
    : name(name), args(std::move(args)) {}

Function::Function(std::unique_ptr<Prototype> proto,
                   std::unique_ptr<expr::ExprNode> body)
    : proto(std::move(proto)), body(std::move(body)) {}
} // namespace ast
