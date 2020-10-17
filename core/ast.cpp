#include "ast.h"

namespace ast {

llvm::Function *getFunction(const std::string &name, llvm::LLVMContext &context,
                            llvm::IRBuilder<> &builder,
                            std::unique_ptr<llvm::Module> &llvmModule,
                            named_values_t &namedValues,
                            function_protos_t &functionProtos) {
  if (auto *f = llvmModule->getFunction(name)) {
    return f;
  }

  auto fI = functionProtos.find(name);
  if (fI != functionProtos.end()) {
    return fI->second->codegen(context, builder, llvmModule, namedValues,
                               functionProtos);
  }

  return nullptr;
}

namespace expr {
Number::Number(double val) : val(val) {}

llvm::Value *Number::codegen(llvm::LLVMContext &context,
                             llvm::IRBuilder<> &builder,
                             std::unique_ptr<llvm::Module> &llvmModule,
                             named_values_t &namedValues,
                             function_protos_t &functionProtos) {
  return llvm::ConstantFP::get(context, llvm::APFloat(val));
}

std::ostream &operator<<(std::ostream &out, const Number &expr) {
  out << "Number{" << expr.val << '}';
  return out;
}

Variable::Variable(const std::string &name) : name(name) {}

llvm::Value *Variable::codegen(llvm::LLVMContext &context,
                               llvm::IRBuilder<> &builder,
                               std::unique_ptr<llvm::Module> &llvmModule,
                               named_values_t &namedValues,
                               function_protos_t &functionProtos) {
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
                             std::unique_ptr<llvm::Module> &llvmModule,
                             named_values_t &namedValues,
                             function_protos_t &functionProtos) {
  auto codegenVisitor = [&](auto &val) {
    return val.codegen(context, builder, llvmModule, namedValues,
                       functionProtos);
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
                           std::unique_ptr<llvm::Module> &llvmModule,
                           named_values_t &namedValues,
                           function_protos_t &functionProtos) {
  llvm::Function *calleeF = getFunction(callee, context, builder, llvmModule,
                                        namedValues, functionProtos);

  if (!calleeF) {
    throw std::runtime_error("Unknown function referenced");
  }

  if (calleeF->arg_size() != args.size()) {
    throw std::runtime_error("Incorrect # arguments passed");
  }

  std::vector<llvm::Value *> argsV;
  std::transform(args.begin(), args.end(), std::back_inserter(argsV),
                 [&](const std::unique_ptr<ExprNode> &arg) {
                   return std::visit(
                       [&](auto &node) {
                         return node.codegen(context, builder, llvmModule,
                                             namedValues, functionProtos);
                       },
                       *arg);
                 });

  for (const auto &arg : argsV) {
    std::cout << arg->getName().str() << '\n';
    ;
  }

  return builder.CreateCall(calleeF, argsV, "calltmp");
}

std::ostream &operator<<(std::ostream &out, const Call &call) {
  out << "Call{TBD}";
  return out;
}
} // namespace expr

Prototype::Prototype(const std::string &name, std::vector<std::string> args,
                     bool isExtern)
    : name(name), args(std::move(args)), isExtern(isExtern) {}

llvm::Function *Prototype::codegen(llvm::LLVMContext &context,
                                   llvm::IRBuilder<> &builder,
                                   std::unique_ptr<llvm::Module> &llvmModule,
                                   named_values_t &namedValues,
                                   function_protos_t &functionProtos) {
  std::vector<llvm::Type *> doubles(args.size(),
                                    llvm::Type::getDoubleTy(context));

  llvm::FunctionType *fT =
      llvm::FunctionType::get(llvm::Type::getDoubleTy(context), doubles, false);

  llvm::Function *f = llvm::Function::Create(
      fT, llvm::Function::ExternalLinkage, name, llvmModule.get());

  size_t i = 0;
  for (auto &arg : f->args()) {
    arg.setName(args[i++]);
  }

  if (isExtern) {
    functionProtos[name] = std::make_unique<Prototype>(*this);
  }

  return f;
}

Function::Function(std::unique_ptr<Prototype> proto,
                   std::unique_ptr<expr::ExprNode> body)
    : proto(std::move(proto)), body(std::move(body)) {}

llvm::Function *Function::codegen(llvm::LLVMContext &context,
                                  llvm::IRBuilder<> &builder,
                                  std::unique_ptr<llvm::Module> &llvmModule,
                                  named_values_t &namedValues,
                                  function_protos_t &functionProtos) {
  auto &p = *proto;
  functionProtos[proto->name] = std::move(proto);

  llvm::Function *function = getFunction(p.name, context, builder, llvmModule,
                                         namedValues, functionProtos);
  if (!function) {
    throw std::runtime_error("failed to generate function prototype");
  }

  if (!function->empty()) {
    throw std::runtime_error("Function cannot be redefined");
  }

  llvm::BasicBlock *bB = llvm::BasicBlock::Create(context, "entry", function);
  builder.SetInsertPoint(bB);

  namedValues.clear();
  for (auto &arg : function->args()) {
    namedValues[arg.getName().str()] = &arg;
  }

  try {
    llvm::Value *retVal = std::visit(
        [&](auto &ret) {
          return ret.codegen(context, builder, llvmModule, namedValues,
                             functionProtos);
        },
        *body);
    builder.CreateRet(retVal);
    llvm::verifyFunction(*function);
    return function;
  } catch (const std::exception &e) {
    function->eraseFromParent();
    throw e;
  }
}
} // namespace ast
