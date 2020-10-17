#ifndef AST_H_
#define AST_H_

#include <iostream>
#include <memory>
#include <variant>
#include <vector>
#include <unordered_map>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

namespace ast {
using named_values_t = std::unordered_map<std::string, llvm::Value *>;

class Prototype;
using function_protos_t =
    std::unordered_map<std::string, std::unique_ptr<Prototype>>;

llvm::Function *
getFunction(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
            std::unique_ptr<llvm::Module> &llvmModule,
            named_values_t &namedValues, function_protos_t &functionProtos,
            std::unique_ptr<llvm::legacy::FunctionPassManager> &passes);

namespace expr {
class Number;
class Variable;
class Binary;
class Call;

using ExprNode = std::variant<Number, Variable, Binary, Call>;

class ExprInterface {
public:
  virtual ~ExprInterface() {}
  virtual llvm::Value *codegen(llvm::LLVMContext &context,
                               llvm::IRBuilder<> &builder,
                               std::unique_ptr<llvm::Module> &llvmModule,
                               named_values_t &namedValues,
                               function_protos_t &functionProtos) = 0;
};

class Number : public ExprInterface {
public:
  Number(double val);

  virtual llvm::Value *codegen(llvm::LLVMContext &context,
                               llvm::IRBuilder<> &builder,
                               std::unique_ptr<llvm::Module> &llvmModule,
                               named_values_t &namedValues,
                               function_protos_t &functionProtos) override;

  friend std::ostream &operator<<(std::ostream &out, const Number &expr);

  double val;
};

class Variable : public ExprInterface {
public:
  Variable(const std::string &name);

  virtual llvm::Value *codegen(llvm::LLVMContext &context,
                               llvm::IRBuilder<> &builder,
                               std::unique_ptr<llvm::Module> &llvmModule,
                               named_values_t &namedValues,
                               function_protos_t &functionProtos) override;

  friend std::ostream &operator<<(std::ostream &out, const Variable &var);

  std::string name;
};

class Binary : public ExprInterface {
public:
  Binary(char op, std::unique_ptr<ExprNode> lhs, std::unique_ptr<ExprNode> rhs);

  virtual llvm::Value *codegen(llvm::LLVMContext &context,
                               llvm::IRBuilder<> &builder,
                               std::unique_ptr<llvm::Module> &llvmModule,
                               named_values_t &namedValues,
                               function_protos_t &functionProtos) override;

  char op;
  std::unique_ptr<ExprNode> lhs;
  std::unique_ptr<ExprNode> rhs;

  friend std::ostream &operator<<(std::ostream &out, const Binary &op);
};

class Call : public ExprInterface {
public:
  Call(const std::string &callee, std::vector<std::unique_ptr<ExprNode>> args);

  virtual llvm::Value *codegen(llvm::LLVMContext &context,
                               llvm::IRBuilder<> &builder,
                               std::unique_ptr<llvm::Module> &llvmModule,
                               named_values_t &namedValues,
                               function_protos_t &functionProtos) override;

  std::string callee;
  std::vector<std::unique_ptr<ExprNode>> args;

  friend std::ostream &operator<<(std::ostream &out, const Call &call);
};
} // namespace expr

class Prototype {
public:
  Prototype(const std::string &name, std::vector<std::string> args,
            bool isExtern = false);

  llvm::Function *codegen(llvm::LLVMContext &context,
                          llvm::IRBuilder<> &builder,
                          std::unique_ptr<llvm::Module> &llvmModule,
                          named_values_t &namedValues,
                          function_protos_t &functionProtos);

  std::string name;
  std::vector<std::string> args;
  bool isExtern;
};

class Function {
public:
  Function(std::unique_ptr<Prototype> proto,
           std::unique_ptr<expr::ExprNode> body);

  llvm::Function *codegen(llvm::LLVMContext &context,
                          llvm::IRBuilder<> &builder,
                          std::unique_ptr<llvm::Module> &llvmModule,
                          named_values_t &namedValues,
                          function_protos_t &functionProtos);

  std::unique_ptr<Prototype> proto;
  std::unique_ptr<expr::ExprNode> body;
};

using AstNode = std::variant<Prototype, Function>;
} // namespace ast

#endif // !AST_H_
