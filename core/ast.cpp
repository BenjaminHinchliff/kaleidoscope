#include "ast.hpp"

namespace ast {

namespace expr {
Number::Number(double val) : val(val) {}

std::ostream &operator<<(std::ostream &out, const Number &expr) {
  out << "Number{" << expr.val << '}';
  return out;
}

Variable::Variable(const std::string &name) : name(name) {}

std::ostream &operator<<(std::ostream &out, const Variable &var) {
  out << "Variable{" << var.name << "}";
  return out;
}

std::ostream &operator<<(std::ostream &out, const Binary &op) {
  out << "Binary{"
      << "lhs: ";
  std::visit([&](const auto &tkn) { out << tkn; }, *op.lhs);
  out << ", op: " << op.op << ", rhs: ";
  std::visit([&](const auto &tkn) { out << tkn; }, *op.rhs);
  out << '}';
  return out;
}

Call::Call(const std::string &callee,
           std::vector<std::unique_ptr<ExprNode>> args)
    : callee(callee), args(std::move(args)) {}

std::ostream &operator<<(std::ostream &out, const Call &call) {
  out << "Call{TBD}";
  return out;
}
} // namespace expr

Prototype::Prototype(const std::string &name, std::vector<std::string> args,
                     bool isExtern)
    : name(name), args(std::move(args)), isExtern(isExtern) {}

Function::Function(std::unique_ptr<Prototype> proto,
                   std::unique_ptr<expr::ExprNode> body)
    : proto(std::move(proto)), body(std::move(body)) {}
} // namespace ast
