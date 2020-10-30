set(CORE_SRCS
	"lexer.cpp"
	"tokens.cpp"
	"ast.cpp"
	"parser.cpp"
	"kaleidoscope_jit.cpp"
	"codegen.cpp")

add_library(kaleidoscope ${CORE_SRCS})
mark_as_advanced(CORE_SRCS)

target_include_directories(kaleidoscope PUBLIC "../include")

find_package(LLVM CONFIG REQUIRED)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

target_include_directories(kaleidoscope PUBLIC ${LLVM_INCLUDE_DIRS})
target_compile_definitions(kaleidoscope PUBLIC ${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs
	Analysis
	Core
	ExecutionEngine
	InstCombine
	Object
	OrcJIT
	RuntimeDyld
	ScalarOpts
	Support
	native)

message(STATUS "Using libs ${llvm_libs}")

target_link_libraries(kaleidoscope PUBLIC ${llvm_libs})

