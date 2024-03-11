#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

// Все фикстуры должны наследоваться от testing::test, и весь код (поля и методы
// SetUp/TearDown) должны быть protected
class VectorTest : public testing::Test {
protected:
  void SetUp() override {
    v_1_.resize(5);
    v_2_.assign(5, 1);
  }

  // void TearDown() override {}

  std::vector<int> v_0_{};
  std::vector<int> v_1_{};
  std::vector<int> v_2_{};
};

TEST_F(VectorTest, TestInitiallyEmpty) {
  ASSERT_THAT(v_0_, testing::IsEmpty());
  v_0_.resize(10);
}

TEST_F(VectorTest, TestResize) {
  EXPECT_THAT(v_0_, testing::IsEmpty());
  EXPECT_THAT(v_1_, testing::SizeIs(5));
  EXPECT_THAT(v_1_, testing::Each(0));
}

TEST_F(VectorTest, TestAssign) {
  EXPECT_THAT(v_2_, testing::SizeIs(5));
  EXPECT_THAT(v_2_, testing::Each(1));
}
