#ifndef CODEGEN_HPP_
#define CODEGEN_HPP_

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"

#include "ast.hpp"

namespace codegen {
class Generator {
public:
  Generator();
private:
  llvm::LLVMContext context;
  llvm::IRBuilder<> builder{context};
  ast::named_values_t namedValues;
  ast::function_protos_t functionProtos;
};
} // namespace codegen

#endif // !CODEGEN_HPP_
