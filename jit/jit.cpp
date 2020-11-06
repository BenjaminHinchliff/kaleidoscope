#include <iostream>
#include <sstream>
#include <string>

#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"

#include "kaleidoscope_jit.hpp"
#include "parser.hpp"

using jit_ptr_t = llvm::orc::KaleidoscopeJIT;

namespace legacy = llvm::legacy;

void makeModule(ast::GenState &state, jit_ptr_t &jit) {
  state.llvmModule =
      std::make_unique<llvm::Module>("KaleidoscopeJIT", state.context);
  state.llvmModule->setDataLayout(jit.getTargetMachine().createDataLayout());
  state.optPasses =
      std::make_unique<legacy::FunctionPassManager>(state.llvmModule.get());
  state.optPasses->add(llvm::createInstructionCombiningPass());
  state.optPasses->add(llvm::createReassociatePass());
  state.optPasses->add(llvm::createGVNPass());
  state.optPasses->add(llvm::createCFGSimplificationPass());
  state.optPasses->doInitialization();
}

void parseAndExecuteTokenStream(lexer::Lexer &lexer,
                                const parser::Parser &parser,
                                llvm::orc::KaleidoscopeJIT &jit,
                                ast::GenState &state) {
  while (!std::holds_alternative<tokens::Eof>(lexer.peek())) {
    auto ast = parser.parse(lexer);

    makeModule(state, jit);
    auto fnIR = std::visit([&](auto &ast) { return ast.codegen(state); }, *ast);

    std::cout << "IR:\n";
    fnIR->print(llvm::outs(), nullptr);

    auto modHandle = jit.addModule(std::move(state.llvmModule));

    auto exprSymbol = jit.findSymbol("__anon_expr");
    if (exprSymbol) {
      double (*fP)() = reinterpret_cast<double (*)()>(
          static_cast<intptr_t>(llvm::cantFail(exprSymbol.getAddress())));
      std::cout << "Eval:\n" << fP() << '\n';
      jit.removeModule(modHandle);
    }
  }
}

int main() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  ast::GenState state;
  llvm::orc::KaleidoscopeJIT jit;

  parser::Parser parser;

  while (true) {
    try {
      std::cout << "ready> ";
      std::string source;
      std::getline(std::cin, source);
      if (source == "exit") {
        break;
      }
      std::stringstream sourceStream(source);
      lexer::Lexer lexer{sourceStream};

      parseAndExecuteTokenStream(lexer, parser, jit, state);
    } catch (const std::exception &e) {
      std::cerr << e.what() << '\n';
    }
  }

  return 0;
}
