#include <safe_cpp/diagnostics/diagnostics.hpp>
#include <safe_cpp/result/result.hpp>

#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace safe::result::tests
{
Result<int, std::string> parse(bool success)
{
    if (!success)
    {
        return Result<int, std::string>::err("failed");
    }

    return Result<int, std::string>::ok(42);
}

Result<int, std::string> doubled(bool success)
{
    SAFE_CPP_TRY(value, parse(success));
    return Result<int, std::string>::ok(value * 2);
}

Result<void, std::string> finish(bool success)
{
    if (!success)
    {
        return Result<void, std::string>::err("failed");
    }

    return Result<void, std::string>::ok();
}

Result<void, std::string> afterFinish(bool success)
{
    SAFE_CPP_TRY_VOID(finish(success));
    return Result<void, std::string>::ok();
}

TEST(ResultTest, StoresSuccess)
{
    auto value = parse(true);
    EXPECT_TRUE(value);
    EXPECT_EQ(value.value(), 42);
}

TEST(ResultTest, StoresError)
{
    auto value = parse(false);
    EXPECT_FALSE(value);
    EXPECT_EQ(value.error(), "failed");
}

TEST(ResultTest, RejectsWrongVariantAccess)
{
    auto value = parse(false);
    EXPECT_THROW(value.value(), diagnostics::Violation);
}

TEST(ResultTest, RejectsErrorAccessOnSuccess)
{
    auto value = parse(true);
    EXPECT_THROW(value.error(), diagnostics::Violation);
}

TEST(ResultTest, StoresVoidSuccess)
{
    auto value = Result<void, std::string>::ok();
    EXPECT_TRUE(value);
    value.value();
}

TEST(ResultTest, StoresVoidError)
{
    auto value = Result<void, std::string>::err("failed");
    EXPECT_FALSE(value);
    EXPECT_EQ(value.error(), "failed");
    EXPECT_THROW(value.value(), diagnostics::Violation);
}

TEST(ResultTest, MapsSuccessValues)
{
    const auto value = Result<int, std::string>::ok(21);

    // clang-format off
    const auto mapped = value.map([](const int number)
    {
        return number * 2;
    });
    // clang-format on

    EXPECT_EQ(mapped.value(), 42);
}

TEST(ResultTest, MapsSuccessToVoid)
{
    const auto value = Result<int, std::string>::ok(42);
    bool seen = false;

    // clang-format off
    const auto mapped = value.map([&seen](const int)
    {
        seen = true;
    });
    // clang-format on

    EXPECT_TRUE(seen);
    EXPECT_TRUE(mapped);
    mapped.value();
}

TEST(ResultTest, MapsErrorValues)
{
    const auto value = Result<int, std::string>::err("failed");

    // clang-format off
    const auto mapped = value.map([](const int number)
    {
        return number * 2;
    });
    // clang-format on

    EXPECT_FALSE(mapped);
    EXPECT_EQ(mapped.error(), "failed");
}

TEST(ResultTest, MovesStoredError)
{
    auto value = Result<int, std::string>::err("failed");
    const std::string error = std::move(value).error();
    EXPECT_EQ(error, "failed");
}

TEST(ResultTest, TryUnwrapsValue)
{
    auto value = doubled(true);
    EXPECT_TRUE(value);
    EXPECT_EQ(value.value(), 84);
}

TEST(ResultTest, TryPropagatesError)
{
    auto value = doubled(false);
    EXPECT_FALSE(value);
    EXPECT_EQ(value.error(), "failed");
}

TEST(ResultTest, MapsVoidSuccess)
{
    const auto value = Result<void, std::string>::ok();

    // clang-format off
    const auto mapped = value.map([]()
    {
        return 42;
    });
    // clang-format on

    EXPECT_EQ(mapped.value(), 42);
}

TEST(ResultTest, MapsVoidSuccessToVoid)
{
    const auto value = Result<void, std::string>::ok();
    bool seen = false;

    // clang-format off
    const auto mapped = value.map([&seen]()
    {
        seen = true;
    });
    // clang-format on

    EXPECT_TRUE(seen);
    EXPECT_TRUE(mapped);
    mapped.value();
}

TEST(ResultTest, MapsVoidError)
{
    const auto value = Result<void, std::string>::err("failed");

    // clang-format off
    const auto mapped = value.map([]()
    {
        return 42;
    });
    // clang-format on

    EXPECT_FALSE(mapped);
    EXPECT_EQ(mapped.error(), "failed");
}

TEST(ResultTest, MovesVoidError)
{
    auto value = Result<void, std::string>::err("failed");
    const std::string error = std::move(value).error();
    EXPECT_EQ(error, "failed");
}

TEST(ResultTest, TryVoidUnwrapsSuccess)
{
    EXPECT_TRUE(afterFinish(true));
}

TEST(ResultTest, TryVoidPropagatesError)
{
    auto value = afterFinish(false);
    EXPECT_FALSE(value);
    EXPECT_EQ(value.error(), "failed");
}
} // namespace safe::result::tests
