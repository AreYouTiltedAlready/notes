cmake_minimum_required(VERSION 3.28)
project(gtest_examples LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(googletest)
enable_testing()

add_executable(
  ${PROJECT_NAME}
  code/test_simple.cc
  code/test_factorial.cc
  code/test_vector.cc
  code/test_parametrized.cc
  code/test_parametrized_with_naming.cc
  code/test_with_assertion_result.cc
)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
  GTest::gtest_main
  gmock
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
