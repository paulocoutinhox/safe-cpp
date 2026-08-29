#include <safe_cpp/arithmetic/checked_integer.hpp>
#include <safe_cpp/diagnostics/diagnostics.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>

namespace safe::arithmetic::tests
{
TEST(ArithmeticTest, AddsWithoutOverflow)
{
    const CheckedInteger<int> left(20);
    const CheckedInteger<int> right(22);
    EXPECT_EQ((left + right).value(), 42);
}

TEST(ArithmeticTest, SubtractsWithoutOverflow)
{
    const CheckedInteger<int> left(50);
    const CheckedInteger<int> right(8);
    EXPECT_EQ((left - right).value(), 42);
}

TEST(ArithmeticTest, MultipliesWithoutOverflow)
{
    const CheckedInteger<int> left(6);
    const CheckedInteger<int> right(7);
    EXPECT_EQ((left * right).value(), 42);
}

TEST(ArithmeticTest, DetectsOverflow)
{
    const CheckedInteger<int> maximum(std::numeric_limits<int>::max());
    const CheckedInteger<int> one(1);
    EXPECT_THROW(static_cast<void>(maximum + one), diagnostics::Violation);
}

TEST(ArithmeticTest, DetectsSubtractionOverflow)
{
    const CheckedInteger<int> minimum(std::numeric_limits<int>::min());
    const CheckedInteger<int> one(1);
    EXPECT_THROW(static_cast<void>(minimum - one), diagnostics::Violation);
}

TEST(ArithmeticTest, DetectsMultiplicationOverflow)
{
    const CheckedInteger<int> maximum(std::numeric_limits<int>::max());
    const CheckedInteger<int> two(2);
    EXPECT_THROW(static_cast<void>(maximum * two), diagnostics::Violation);
}

TEST(ArithmeticTest, DetectsDivisionByZero)
{
    const CheckedInteger<int> value(42);
    const CheckedInteger<int> zero(0);
    EXPECT_THROW(static_cast<void>(value / zero), diagnostics::Violation);
}

TEST(ArithmeticTest, DetectsSignedDivisionOverflow)
{
    const CheckedInteger<int> minimum(std::numeric_limits<int>::min());
    const CheckedInteger<int> negativeOne(-1);
    EXPECT_THROW(static_cast<void>(minimum / negativeOne), diagnostics::Violation);
}

TEST(ArithmeticTest, AddsWideSignedValues)
{
    const CheckedInteger<std::int64_t> left(20);
    const CheckedInteger<std::int64_t> right(22);
    EXPECT_EQ((left + right).value(), 42);
}

TEST(ArithmeticTest, AddsUnsignedValues)
{
    const CheckedInteger<unsigned> left(20u);
    const CheckedInteger<unsigned> right(22u);
    EXPECT_EQ((left + right).value(), 42u);
}

TEST(ArithmeticTest, DetectsUnsignedOverflow)
{
    const CheckedInteger<unsigned> maximum(std::numeric_limits<unsigned>::max());
    const CheckedInteger<unsigned> one(1u);
    EXPECT_THROW(static_cast<void>(maximum + one), diagnostics::Violation);
}

TEST(ArithmeticTest, DetectsUnsignedUnderflow)
{
    const CheckedInteger<unsigned> zero(0u);
    const CheckedInteger<unsigned> one(1u);
    EXPECT_THROW(static_cast<void>(zero - one), diagnostics::Violation);
}

TEST(ArithmeticTest, DetectsUnsignedMultiplicationOverflow)
{
    const CheckedInteger<unsigned> left((std::numeric_limits<unsigned>::max() / 2u) + 1u);
    const CheckedInteger<unsigned> two(2u);
    EXPECT_THROW(static_cast<void>(left * two), diagnostics::Violation);
}

TEST(ArithmeticTest, RemainderWithoutOverflow)
{
    const CheckedInteger<int> left(47);
    const CheckedInteger<int> right(5);
    EXPECT_EQ((left % right).value(), 2);
}

TEST(ArithmeticTest, DetectsRemainderByZero)
{
    const CheckedInteger<int> value(42);
    const CheckedInteger<int> zero(0);
    EXPECT_THROW(static_cast<void>(value % zero), diagnostics::Violation);
}

TEST(ArithmeticTest, DetectsSignedRemainderOverflow)
{
    const CheckedInteger<int> minimum(std::numeric_limits<int>::min());
    const CheckedInteger<int> negativeOne(-1);
    EXPECT_THROW(static_cast<void>(minimum % negativeOne), diagnostics::Violation);
}

TEST(ArithmeticTest, NegatesWithoutOverflow)
{
    const CheckedInteger<int> value(42);
    EXPECT_EQ((-value).value(), -42);
}

TEST(ArithmeticTest, DetectsSignedNegationOverflow)
{
    const CheckedInteger<int> minimum(std::numeric_limits<int>::min());
    EXPECT_THROW(static_cast<void>(-minimum), diagnostics::Violation);
}

TEST(ArithmeticTest, DetectsUnsignedNegationOverflow)
{
    const CheckedInteger<unsigned> value(1u);
    EXPECT_THROW(static_cast<void>(-value), diagnostics::Violation);
}
} // namespace safe::arithmetic::tests
