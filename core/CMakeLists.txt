add_library(Kaleidoscope "lexer.cpp" "tokens.cpp" "ast.cpp" "parser.cpp")

target_include_directories(Kaleidoscope PUBLIC "../include")

find_package(LLVM CONFIG REQUIRED)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

target_include_directories(Kaleidoscope PUBLIC ${LLVM_INCLUDE_DIRS})
target_compile_definitions(Kaleidoscope PUBLIC ${LLVM_DEFINITIONS})

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

target_link_libraries(Kaleidoscope PUBLIC ${llvm_libs})

