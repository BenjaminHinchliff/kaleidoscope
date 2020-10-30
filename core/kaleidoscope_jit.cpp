#include "kaleidoscope_jit.hpp"

namespace llvm {
namespace orc {

KaleidoscopeJIT::KaleidoscopeJIT()
    : resolver(createLegacyLookupResolver(
          es,
          [this](StringRef Name) {
            return findMangledSymbol(std::string(Name));
          },
          [](Error Err) { cantFail(std::move(Err), "lookupFlags failed"); })),
      tm(EngineBuilder().selectTarget()), dl(tm->createDataLayout()),
      objectLayer(AcknowledgeORCv1Deprecation, es,
                  [this](VModuleKey) {
                    return ObjLayerT::Resources{
                        std::make_shared<SectionMemoryManager>(), resolver};
                  }),
      compileLayer(AcknowledgeORCv1Deprecation, objectLayer,
                   SimpleCompiler(*tm)) {
  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

TargetMachine &KaleidoscopeJIT::getTargetMachine() { return *tm; }

VModuleKey KaleidoscopeJIT::addModule(std::unique_ptr<Module> M) {
  auto K = es.allocateVModule();
  cantFail(compileLayer.addModule(K, std::move(M)));
  moduleKeys.push_back(K);
  return K;
}

void KaleidoscopeJIT::removeModule(VModuleKey K) {
  moduleKeys.erase(find(moduleKeys, K));
  cantFail(compileLayer.removeModule(K));
}

JITSymbol KaleidoscopeJIT::findSymbol(const std::string Name) {
  return findMangledSymbol(mangle(Name));
}

std::string KaleidoscopeJIT::mangle(const std::string &Name) {
  std::string MangledName;
  {
    raw_string_ostream MangledNameStream(MangledName);
    Mangler::getNameWithPrefix(MangledNameStream, Name, dl);
  }
  return MangledName;
}

JITSymbol KaleidoscopeJIT::findMangledSymbol(const std::string &Name) {
#ifdef _WIN32
  // The symbol lookup of ObjectLinkingLayer uses the SymbolRef::SF_Exported
  // flag to decide whether a symbol will be visible or not, when we call
  // IRCompileLayer::findSymbolIn with ExportedSymbolsOnly set to true.
  //
  // But for Windows COFF objects, this flag is currently never set.
  // For a potential solution see: https://reviews.llvm.org/rL258665
  // For now, we allow non-exported symbols on Windows as a workaround.
  const bool ExportedSymbolsOnly = false;
#else
  const bool ExportedSymbolsOnly = true;
#endif

  // Search modules in reverse order: from last added to first added.
  // This is the opposite of the usual search order for dlsym, but makes more
  // sense in a REPL where we want to bind to the newest available definition.
  for (auto H : make_range(moduleKeys.rbegin(), moduleKeys.rend()))
    if (auto Sym = compileLayer.findSymbolIn(H, Name, ExportedSymbolsOnly))
      return Sym;

  // If we can't find the symbol in the JIT, try looking in the host process.
  if (auto SymAddr = RTDyldMemoryManager::getSymbolAddressInProcess(Name))
    return JITSymbol(SymAddr, JITSymbolFlags::Exported);

#ifdef _WIN32
  // For Windows retry without "_" at beginning, as RTDyldMemoryManager uses
  // GetProcAddress and standard libraries like msvcrt.dll use names
  // with and without "_" (for example "_itoa" but "sin").
  if (Name.length() > 2 && Name[0] == '_')
    if (auto SymAddr =
            RTDyldMemoryManager::getSymbolAddressInProcess(Name.substr(1)))
      return JITSymbol(SymAddr, JITSymbolFlags::Exported);
#endif

  return nullptr;
}

} // namespace orc
} // namespace llvm
