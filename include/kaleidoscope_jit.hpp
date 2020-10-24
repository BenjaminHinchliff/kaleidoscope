#ifndef KALEIDOSCOPEJIT_HPP_
#define KALEIDOSCOPEJIT_HPP_

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace llvm {
namespace orc {

class KaleidoscopeJIT {
public:
  using ObjLayerT = LegacyRTDyldObjectLinkingLayer;
  using CompileLayerT = LegacyIRCompileLayer<ObjLayerT, SimpleCompiler>;

  KaleidoscopeJIT();
  TargetMachine &getTargetMachine();
  VModuleKey addModule(std::unique_ptr<Module> M);
  void removeModule(VModuleKey K);
  JITSymbol findSymbol(const std::string Name);

private:
  std::string mangle(const std::string &Name);
  JITSymbol findMangledSymbol(const std::string &Name);

  ExecutionSession ES;
  std::shared_ptr<SymbolResolver> Resolver;
  std::unique_ptr<TargetMachine> TM;
  const DataLayout DL;
  ObjLayerT ObjectLayer;
  CompileLayerT CompileLayer;
  std::vector<VModuleKey> ModuleKeys;
};

} // end namespace orc
} // end namespace llvm

#endif // KALEIDOSCOPEJIT_HPP_
