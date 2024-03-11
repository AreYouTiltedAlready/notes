#include <gtest/gtest.h>

#include <format>

TEST(DummyTests, TestFormat) {
  constexpr std::string_view kPrefix = "Hello";
  EXPECT_EQ(std::format("{}, {}!", kPrefix, "Nazar"), "Hello, Nazar!");
}
