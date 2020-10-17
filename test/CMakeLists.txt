cmake_minimum_required(VERSION 3.8)

add_executable(tests "lexer.cpp" "parser.cpp")

target_link_libraries(tests PUBLIC Kaleidoscope)

target_link_libraries(tests PUBLIC GTest::GTest GTest::Main)

include(GoogleTest)
gtest_add_tests(TARGET tests)
