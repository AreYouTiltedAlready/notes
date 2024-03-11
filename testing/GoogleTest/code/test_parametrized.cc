#include <gmock/gmock.h>
#include <gtest/gtest.h>

class ParametrizedTest : public testing::TestWithParam<std::string_view> {};

INSTANTIATE_TEST_SUITE_P(FooBarBaz, ParametrizedTest,
                         testing::Values("foo", "bar", "baz"));

TEST_P(ParametrizedTest, SampleTest) {
  EXPECT_THAT(GetParam(), testing::MatchesRegex(R"~(foo|bar|baz)~"));
}
