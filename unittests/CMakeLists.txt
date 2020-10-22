set(TEST_SRCS
"lexer_unittest.cpp"
"parser_unittest.cpp")

add_executable(tests ${TEST_SRCS})
mark_as_advanced(TEST_SRCS)

target_link_libraries(tests PUBLIC Kaleidoscope)

target_link_libraries(tests PUBLIC GTest::GTest GTest::Main)

include(GoogleTest)
gtest_add_tests(TARGET tests)
