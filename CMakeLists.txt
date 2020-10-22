cmake_minimum_required(VERSION 3.1)

project("Kaleidoscope" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# core library
add_subdirectory("core")
# Just-in-Time Compiler (the only one right now)
add_subdirectory("jit")
# if tests exist add test subdir
find_package(GTest)
if(${GTEST_FOUND})
	enable_testing()
	add_subdirectory("unittests")
endif()
