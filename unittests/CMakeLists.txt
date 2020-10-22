set(TEST_SRCS
"lexer_unittest.cpp"
"parser_unittest.cpp")

add_executable(unittests ${TEST_SRCS})
mark_as_advanced(TEST_SRCS)

target_link_libraries(unittests PUBLIC Kaleidoscope)

target_link_libraries(unittests PUBLIC GTest::GTest GTest::Main)

include(GoogleTest)
gtest_add_tests(TARGET unittests)
