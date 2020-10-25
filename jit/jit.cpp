#include <iostream>
#include <sstream>
#include <string>

#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"

#include "kaleidoscope_jit.hpp"
#include "parser.hpp"

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

using jit_ptr_t = std::unique_ptr<llvm::orc::KaleidoscopeJIT>;

std::unique_ptr<llvm::Module> makeModule(llvm::LLVMContext &context,
                                         const jit_ptr_t &jit) {
  auto llvmMod = std::make_unique<llvm::Module>("KaleidoscopeJIT", context);
  llvmMod->setDataLayout(jit->getTargetMachine().createDataLayout());
  return llvmMod;
}

void parseAndExecuteTokenStream(lexer::Lexer &lexer,
                                const parser::Parser &parser,
                                llvm::LLVMContext &context,
                                llvm::IRBuilder<> &builder,
                                ast::named_values_t &namedValues,
                                ast::function_protos_t &functionProtos,
                                jit_ptr_t &jit) {
  auto llvmModule = makeModule(context, jit);

  while (!std::holds_alternative<tokens::Eof>(lexer.peek())) {
    auto ast = parser.parse(lexer);
    auto fnIR =
        std::visit(overloaded{[&](ast::Prototype &ast) {
                                return ast.codegen(context, builder, llvmModule,
                                                   namedValues, functionProtos);
                              },
                              [&](ast::Function &ast) {
                                return ast.codegen(context, builder, llvmModule,
                                                   namedValues, functionProtos);
                              }},
                   *ast);

    std::cout << "IR:\n";
    fnIR->print(llvm::outs(), nullptr);

    auto modHandle = jit->addModule(std::move(llvmModule));

    auto exprSymbol = jit->findSymbol("__anon_expr");
    if (exprSymbol) {
      double (*fP)() = reinterpret_cast<double (*)()>(
          static_cast<intptr_t>(llvm::cantFail(exprSymbol.getAddress())));
      std::cout << "Eval:\n" << fP() << '\n';
      jit->removeModule(modHandle);
    }
  }
}

int main() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  llvm::LLVMContext context;
  llvm::IRBuilder<> builder{context};
  ast::named_values_t namedValues;
  ast::function_protos_t functionProtos;

  auto jit = std::make_unique<llvm::orc::KaleidoscopeJIT>();

  parser::Parser parser{};

  while (true) {
    try {
      std::cout << "ready> ";
      std::string source;
      std::getline(std::cin, source);
      std::stringstream sourceStream(source);
      lexer::Lexer lexer{sourceStream};

      parseAndExecuteTokenStream(lexer, parser, context, builder, namedValues,
                                 functionProtos, jit);
    } catch (const std::exception &e) {
      std::cerr << e.what() << '\n';
    }
  }

  return 0;
}
