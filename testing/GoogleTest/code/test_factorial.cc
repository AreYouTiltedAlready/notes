#include <gtest/gtest.h>

#include <format>
#include <stdexcept>

int Factorial(int n) {
  if (n < 0) {
    throw std::invalid_argument(
        std::format("Expected: n >= 0, found: n = {}", n));
  }

  int result = 1;
  for (int i = 1; i <= n; ++i) {
    result *= i;
  }
  return result;
}

TEST(FactorialTest, ZeroInput) { ASSERT_EQ(Factorial(0), 1); }

TEST(FactorialTest, PositiveInput) {
  EXPECT_EQ(Factorial(1), 1);
  EXPECT_EQ(Factorial(2), 2);
  EXPECT_EQ(Factorial(3), 6);
  EXPECT_EQ(Factorial(4), 24);
  EXPECT_EQ(Factorial(5), 120);
  EXPECT_EQ(Factorial(6), 720);
}

TEST(FactorialTest, NegativeInput) {
  EXPECT_THROW(Factorial(-1), std::invalid_argument);
}