#include "ast.h"

namespace ast {
namespace expr {
Number::Number(double val) : val(val) {}

llvm::Value *Number::codegen(llvm::LLVMContext &context,
                             llvm::IRBuilder<> &builder,
                             std::shared_ptr<llvm::Module> llvmModule,
                             named_values_t &namedValues) const {
  return llvm::ConstantFP::get(context, llvm::APFloat(val));
}

std::ostream &operator<<(std::ostream &out, const Number &expr) {
  out << "Number{" << expr.val << '}';
  return out;
}

Variable::Variable(const std::string &name) : name(name) {}

llvm::Value *Variable::codegen(llvm::LLVMContext &context,
                               llvm::IRBuilder<> &builder,
                               std::shared_ptr<llvm::Module> llvmModule,
                               named_values_t &namedValues) const {
  llvm::Value *V = namedValues[name];
  if (!V) {
    throw std::runtime_error("Unknown variable name");
  }
  return V;
}

std::ostream &operator<<(std::ostream &out, const Variable &var) {
  out << "Variable{" << var.name << "}";
  return out;
}

Binary::Binary(char op, std::unique_ptr<ExprNode> lhs,
               std::unique_ptr<ExprNode> rhs)
    : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

llvm::Value *Binary::codegen(llvm::LLVMContext &context,
                             llvm::IRBuilder<> &builder,
                             std::shared_ptr<llvm::Module> llvmModule,
                             named_values_t &namedValues) const {
  auto codegenVisitor = [&](const auto &val) {
    return val.codegen(context, builder, llvmModule, namedValues);
  };
  llvm::Value *L = std::visit(codegenVisitor, *lhs);
  llvm::Value *R = std::visit(codegenVisitor, *rhs);
  if (!L || !R) {
    throw std::runtime_error("exceptional failure in binary gen");
  }

  // TODO: maybe not a switch (map of function pointers?)
  switch (op) {
  case L'+':
    return builder.CreateFAdd(L, R, "addtmp");
  case L'-':
    return builder.CreateFSub(L, R, "subtmp");
  case L'*':
    return builder.CreateFMul(L, R, "multmp");
  case L'<':
    L = builder.CreateFCmpULT(L, R, "cmptmp");
    return builder.CreateUIToFP(L, llvm::Type::getDoubleTy(context), "booltmp");
  default:
    throw std::runtime_error("unknown operation!");
  }
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

llvm::Value *Call::codegen(llvm::LLVMContext &context,
                           llvm::IRBuilder<> &builder,
                           std::shared_ptr<llvm::Module> llvmModule,
                           named_values_t &namedValues) const {
  // Function* calleeF = llvmModule->getFunction(callee);
  return nullptr;
}

std::ostream &operator<<(std::ostream &out, const Call &call) {
  out << "Call{TBD}";
  return out;
}
} // namespace expr

Prototype::Prototype(const std::string &name, std::vector<std::string> args)
    : name(name), args(std::move(args)) {}

Function::Function(std::unique_ptr<Prototype> proto,
                   std::unique_ptr<expr::ExprNode> body)
    : proto(std::move(proto)), body(std::move(body)) {}
} // namespace ast
