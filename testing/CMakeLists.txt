# testing: assert/static_assert, a tiny self-contained test
# harness (mini_test.h), and a Calculator with its test suite.
# No framework is downloaded; see mini_test.h for how GoogleTest
# and Catch2 map onto the same ideas.

zh_add_example(testing 01_assert_and_static_assert 01_assert_and_static_assert.cpp)

# The class under test, as a library so tests and programs can share it.
add_library(testing_calculator STATIC calculator.cpp)
target_include_directories(testing_calculator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
zh_apply_settings(testing_calculator)

# The test suite: an ordinary executable that returns non-zero on
# failure, which is all ctest needs.
zh_add_example(testing calculator_test calculator_test.cpp)
target_link_libraries(testing_calculator_test PRIVATE testing_calculator)

zh_add_topic_runner(testing)
