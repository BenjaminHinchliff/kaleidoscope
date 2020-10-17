cmake_minimum_required(VERSION 3.8)

project("Kaleidoscope" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# core library
add_subdirectory("core")
add_subdirectory("jit")
find_package(GTest)
if(${GTEST_FOUND})
	enable_testing()
	add_subdirectory("test")
endif()
