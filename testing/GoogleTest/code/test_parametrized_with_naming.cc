#include <gmock/gmock.h>
#include <gtest/gtest.h>

class ParametrizedTestWithNaming
    : public testing::TestWithParam<std::string_view> {};

constexpr std::string_view kParams[] = {"foo", "bar", "baz"};

INSTANTIATE_TEST_SUITE_P(
    FooBarBazWithNaming, ParametrizedTestWithNaming, testing::ValuesIn(kParams),
    [](const testing::TestParamInfo<ParametrizedTestWithNaming::ParamType>&
           info) { return std::string(info.param); });

TEST_P(ParametrizedTestWithNaming, SampleTest) {
  EXPECT_THAT(GetParam(), testing::MatchesRegex(R"~(foo|bar|baz)~"));
}
