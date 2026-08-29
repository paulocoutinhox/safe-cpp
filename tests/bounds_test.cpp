#include <safe_cpp/bounds/bounds.hpp>
#include <safe_cpp/diagnostics/diagnostics.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <limits>
#include <string_view>

namespace safe::bounds::tests
{
TEST(BoundsTest, VectorChecksEveryIndex)
{
    Vector<int> values{1, 2, 3};
    EXPECT_EQ(values[1], 2);
    EXPECT_THROW(values[3], diagnostics::Violation);
}

TEST(BoundsTest, VectorSupportsGrowth)
{
    Vector<int> values;
    EXPECT_TRUE(values.empty());

    values.reserve(2);
    values.emplace_back(42);

    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values[0], 42);
}

TEST(BoundsTest, ArrayChecksEveryIndex)
{
    Array<int, 2> values(std::array<int, 2>{4, 5});
    EXPECT_EQ(values[0], 4);
    EXPECT_THROW(values[2], diagnostics::Violation);
}

TEST(BoundsTest, ConstArrayChecksEveryIndex)
{
    const Array<int, 2> values(std::array<int, 2>{4, 5});
    EXPECT_EQ(values[1], 5);
    EXPECT_THROW(values[2], diagnostics::Violation);
}

TEST(BoundsTest, SliceChecksEveryIndex)
{
    int values[]{4, 5, 6};
    Slice<int> slice(values);
    EXPECT_EQ(slice[2], 6);
    EXPECT_THROW(slice[3], diagnostics::Violation);
}

TEST(BoundsTest, SliceChecksSubspan)
{
    int values[]{1, 2, 3, 4};
    Slice<int> slice(values);
    const auto inner = slice.subspan(1, 2);

    EXPECT_EQ(inner.size(), 2);
    EXPECT_EQ(inner[0], 2);
    EXPECT_THROW(static_cast<void>(slice.subspan(2, 3)), diagnostics::Violation);
}

TEST(BoundsTest, ConstSliceDoesNotAllowMutation)
{
    int values[]{4, 5, 6};
    const Slice<int> slice(values);
    EXPECT_EQ(slice[1], 5);
    EXPECT_THROW(slice[3], diagnostics::Violation);
}

TEST(BoundsTest, SliceAcceptsEmptySubspanAtEnd)
{
    int values[]{1, 2, 3};
    const Slice<int> slice(values);
    const auto empty = slice.subspan(3, 0);
    EXPECT_TRUE(empty.empty());
}

TEST(BoundsTest, SliceRejectsOverflowingSubspan)
{
    int values[]{1, 2, 3};
    const Slice<int> slice(values);
    EXPECT_THROW(static_cast<void>(slice.subspan(1, std::numeric_limits<std::size_t>::max())), diagnostics::Violation);
}

TEST(BoundsTest, StringChecksEveryIndex)
{
    String value(std::string("safe"));
    EXPECT_EQ(value[0], 's');
    EXPECT_THROW(value[4], diagnostics::Violation);
}

TEST(BoundsTest, ConstStringChecksEveryIndex)
{
    const String value(std::string_view("safe"));
    EXPECT_TRUE(String().empty());
    EXPECT_EQ(value[1], 'a');
    EXPECT_THROW(value[4], diagnostics::Violation);
}
} // namespace safe::bounds::tests
