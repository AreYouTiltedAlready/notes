#include <gmock/gmock.h>
#include <gtest/gtest.h>

testing::AssertionResult IsEven(int n) {
  if (n % 2 == 0) {
    return testing::AssertionSuccess();
  }
  return testing::AssertionFailure() << n << " is odd";
}

TEST(TestWithAssertionResult, SampleTest) {
  EXPECT_TRUE(IsEven(4));
  EXPECT_TRUE(IsEven(5));
}
