# GoogleTest

## Quickstart

### Некоторые общие рекомендации
* Тесты должны быть *независимыми* (дебажить композицию нескольких тестов - удовольствие ниже среднего) и *воспроизводимыми* (тут и так понятно, зачем).
* Тесты должны быть хорошо организованы (сгруппированы в testuitы) и должны отражать структуру кода (инвестиции в читаемость и поддерживаемость)
* Тесты должны быть портируемы
* Тесты должны давать максимум информации об ошибке, если она происходит

### Configure and build
Ну, тут всё просто - достаточно взглянуть на конфигурацию этого проекта (добавляем GoogleTest через git submodules, пишем правильный CMakeLists.txt)  
Если что, [тут](https://google.github.io/googletest/quickstart-cmake.html) рассказывают, как это делать.

### Структура теста
Самый простой пример выглядит так:
```cpp
TEST(TestsuiteName, TestName) {
  // Test code
}
```

### Проверки
Их очень много, вот две главные, остальные будут рассматриваться по мере продвижения:
* `EXPECT_*(...)` - логирует ошибку, если `*` не выполнено, тест продолжается, но вердикт в любом случае `FAILED`
* `ASSERT_*(...)` - то же, что и `EXPECT_*(...)`, но тест не продолжается (fatal error)

### Пример теста
```cpp
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
```

Вот, собственно, пример самодокументируемого чека `EXPECT_THROW(...)`.

## Фикстуры
Та же концепция `SetUp`/`TearDown`, что и в `pytest`. Специфичные для `GoogleTest` моменты:  
* Скоуп фикстуры здесь настраивается руками и чуть более хитро, нежели в `pytest`е
* Тесты, использующие конкретную фикстуру (пусть она называется `TestFixtureName`), объявляются так:
```cpp
TEST_F(TestFixtureName, TestName) {
  ...
}
```
* Все изменения в фикстуре **видны** между тестами (в целом, следствие первого пункта)
* `SetUp` и `TearDown` нужно реализовывать ручками

Впрочем, всё это довольно абстрактно, поэтому лучше привести "живой" пример:

```cpp
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
}

TEST_F(VectorTest, TestResize) {
  EXPECT_THAT(v_1_, testing::SizeIs(5));
  EXPECT_THAT(v_1_, testing::Each(0));
}

TEST_F(VectorTest, TestAssign) {
  EXPECT_THAT(v_2_, testing::SizeIs(5));
  EXPECT_THAT(v_2_, testing::Each(1));
}
```

Вот заодно и краткое введение в матчеры

### Настройка скоупа фикстуры
По умолчанию для каждого теста, использующего некоторую фикстуру, создаётся новый ее инстанс.
Если это слишком дорого, мы можем сделать `SetUp`/`TearDown` один раз для одной testsuite:
```cpp
class FooTest : public testing::Test {
 protected:
  // Per-test-suite set-up.
  // Called before the first test in this test suite.
  // Can be omitted if not needed.
  static void SetUpTestSuite() {
    shared_resource_ = new ...;

    // If `shared_resource_` is **not deleted** in `TearDownTestSuite()`,
    // reallocation should be prevented because `SetUpTestSuite()` may be called
    // in subclasses of FooTest and lead to memory leak.
    //
    // if (shared_resource_ == nullptr) {
    //   shared_resource_ = new ...;
    // }
  }

  // Per-test-suite tear-down.
  // Called after the last test in this test suite.
  // Can be omitted if not needed.
  static void TearDownTestSuite() {
    delete shared_resource_;
    shared_resource_ = nullptr;
  }

  // You can define per-test set-up logic as usual.
  void SetUp() override { ... }

  // You can define per-test tear-down logic as usual.
  void TearDown() override { ... }

  // Some expensive resource shared by all tests.
  static T* shared_resource_;
};

T* FooTest::shared_resource_ = nullptr;

TEST_F(FooTest, Test1) {
  ... you can refer to shared_resource_ here ...
}

TEST_F(FooTest, Test2) {
  ... you can refer to shared_resource_ here ...
}
```
Если и этого мало, можно сделать глобальные `SetUp`, `TearDown`:
```cpp
class Environment : public ::testing::Environment {
 public:
  ~Environment() override {}

  // Override this to define how to set up the environment.
  void SetUp() override {}

  // Override this to define how to tear down the environment.
  void TearDown() override {}
};

testing::Environment* const foo_env =
    testing::AddGlobalTestEnvironment(new FooEnvironment);

// Now use foo_env as you want
```
Еще стоит помнить: в этом случае `GoogleTest` после вызова `testing::AddGlobalTestEnvironment()` захватывает указатель, поэтому удалять его самим не нужно.

Если добавить несколько таких объектов, они создадутся в порядке инициализации и уничтожатся в обратном порядке, но это, как обычно, гарантируется только для одной единицы трансляции, поэтому, если есть шанс, что что-то пойдёт не так, лучше всё же написать свой `main()` и уже там создать все нужные переменные:

```cpp
...

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  // Что-то делаем
  return RUN_ALL_TESTS();
}
```

## Параметризованные тесты
Пример:
```cpp
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class ParametrizedTest : public testing::TestWithParam<std::string_view> {};

INSTANTIATE_TEST_SUITE_P(FooBarBaz, ParametrizedTest,
                         testing::Values("foo", "bar", "baz"));

TEST_P(ParametrizedTest, SampleTest) {
  EXPECT_THAT(GetParam(), testing::MatchesRegex(R"~(foo|bar|baz)~"));
}
```

Тестам можно задавать имена, изначально эти имена - индексы аргументов:
```
[ RUN      ] FooBarBaz/ParametrizedTest.SampleTest/0
[       OK ] FooBarBaz/ParametrizedTest.SampleTest/0 (0 ms)
[ RUN      ] FooBarBaz/ParametrizedTest.SampleTest/1
[       OK ] FooBarBaz/ParametrizedTest.SampleTest/1 (0 ms)
[ RUN      ] FooBarBaz/ParametrizedTest.SampleTest/2
[       OK ] FooBarBaz/ParametrizedTest.SampleTest/2 (0 ms)
```
В общем случае это делается так:
```cpp
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
```

В общем-то, тесты можно параметризовать и по типу (для каких-то шаблонных классов, например)
А ещё, лучше создавать фикстуру в конструкторе и уничтожать в деструкторе (но можно и `SetUp`/`TearDown`)

Ещё полезная штука:
```cpp
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
```
При падении выведется что-то такое, что более информативно, нежели `IsEven(5) is false`
```
Value of: IsEven(5)
  Actual: false (5 is odd)
Expected: true
```

Есть ещё много-много всего, на самом деле, тут описан лишь необходимый минимум.

Источники: [Дока](https://google.github.io/googletest/) по GoogleTest
