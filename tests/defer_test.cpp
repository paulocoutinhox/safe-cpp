#include <safe_cpp/defer/defer.hpp>

#include <gtest/gtest.h>

#include <stdexcept>

namespace safe::scope::tests
{
TEST(DeferTest, RunsAtScopeExit)
{
    int value = 0;

    {
        // clang-format off
        defer
        {
            value = 42;
        };
        // clang-format on

        EXPECT_EQ(value, 0);
    }

    EXPECT_EQ(value, 42);
}

TEST(DeferTest, CanBeCancelled)
{
    int value = 0;

    // clang-format off
    auto finalizer = makeDefer([&value]()
    {
        value = 42;
    });
    // clang-format on

    finalizer.cancel();
    EXPECT_FALSE(finalizer.active());
    EXPECT_EQ(value, 0);
}

TEST(DeferTest, RunsOnExceptionUnwind)
{
    int value = 0;

    try
    {
        // clang-format off
        defer
        {
            value = 42;
        };
        // clang-format on

        throw std::runtime_error("Scope failed");
    }
    catch (const std::runtime_error &)
    {
    }

    EXPECT_EQ(value, 42);
}
} // namespace safe::scope::tests
