#include <safe_cpp/diagnostics/diagnostics.hpp>
#include <safe_cpp/memory/memory.hpp>

#include <gtest/gtest.h>

#include <memory>

namespace safe::memory::tests
{
TEST(MemoryTest, BoxOwnsOneValue)
{
    auto value = Box<int>::make(42);
    EXPECT_EQ(*value, 42);
}

TEST(MemoryTest, BoxReleasesUniqueOwnership)
{
    auto value = Box<int>::make(42);
    const auto unique = std::move(value).intoUnique();
    EXPECT_EQ(*unique, 42);
    EXPECT_THROW(static_cast<void>(*value), diagnostics::Violation);
}

TEST(MemoryTest, RejectsMovedFromBox)
{
    auto first = Box<int>::make(42);
    auto second = std::move(first);
    EXPECT_EQ(*second, 42);
    EXPECT_THROW(static_cast<void>(*first), diagnostics::Violation);
}

TEST(MemoryTest, ArcKeepsSharedOwnership)
{
    auto value = Arc<int>::make(42);
    auto copy = value;
    EXPECT_EQ(*copy, 42);
    EXPECT_EQ(value.use_count(), 2);
}

TEST(MemoryTest, ArcRejectsNullSharedPointer)
{
    EXPECT_THROW(static_cast<void>(Arc<int>::fromShared(nullptr)), diagnostics::Violation);
}

TEST(MemoryTest, RejectsMovedFromArc)
{
    auto first = Arc<int>::make(42);
    auto second = std::move(first);
    EXPECT_EQ(*second, 42);
    EXPECT_THROW(static_cast<void>(*first), diagnostics::Violation);
}

TEST(MemoryTest, WeakExpiresAfterLastOwner)
{
    Weak<int> weak;

    {
        auto value = Arc<int>::make(42);
        weak = value.downgrade();
        EXPECT_TRUE(weak.upgrade());
    }

    EXPECT_TRUE(weak.expired());
    EXPECT_FALSE(weak.upgrade());
}

TEST(MemoryTest, NonNullWrapsReference)
{
    int number = 42;
    const NonNull<int> pointer(number);
    EXPECT_EQ(*pointer, 42);
    EXPECT_EQ(pointer.get(), 42);
}

TEST(MemoryTest, NonNullFromPointer)
{
    int number = 42;
    const auto pointer = NonNull<int>::from(&number);
    EXPECT_TRUE(pointer);
    EXPECT_EQ(*pointer.value(), 42);
}

TEST(MemoryTest, NonNullFromRejectsNull)
{
    EXPECT_FALSE(NonNull<int>::from(nullptr));
}

TEST(MemoryTest, NonNullRejectsNull)
{
    EXPECT_THROW((NonNull<int>(static_cast<int *>(nullptr))), diagnostics::Violation);
}
} // namespace safe::memory::tests
