#include <safe_cpp/diagnostics/diagnostics.hpp>
#include <safe_cpp/option/option.hpp>

#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace safe::option::tests
{
TEST(OptionTest, StoresAValue)
{
    auto value = Option<int>::some(42);
    EXPECT_TRUE(value.hasValue());
    EXPECT_EQ(value.value(), 42);
}

TEST(OptionTest, RejectsEmptyAccess)
{
    auto value = Option<int>::none();
    EXPECT_THROW(value.value(), diagnostics::Violation);
}

TEST(OptionTest, ReturnsFallbackWhenEmpty)
{
    EXPECT_EQ(Option<int>::none().valueOr(42), 42);
    EXPECT_EQ(Option<int>::some(7).valueOr(42), 7);
}

TEST(OptionTest, MapsPresentValues)
{
    const auto value = Option<int>::some(21);

    // clang-format off
    const auto mapped = value.map([](const int number)
    {
        return number * 2;
    });
    // clang-format on

    EXPECT_EQ(mapped.value(), 42);
}

TEST(OptionTest, MapsEmptyValues)
{
    const auto value = Option<int>::none();

    // clang-format off
    const auto mapped = value.map([](const int number)
    {
        return number * 2;
    });
    // clang-format on

    EXPECT_FALSE(mapped);
}

TEST(OptionTest, MovesStoredValue)
{
    auto value = Option<std::string>::some("safe");
    const std::string moved = std::move(value).value();
    EXPECT_EQ(moved, "safe");
}
} // namespace safe::option::tests
