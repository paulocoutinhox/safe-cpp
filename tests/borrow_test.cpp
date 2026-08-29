#include <safe_cpp/borrow/borrow.hpp>
#include <safe_cpp/diagnostics/diagnostics.hpp>

#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <utility>
#include <vector>

namespace safe::borrow::tests
{
TEST(BorrowTest, AllowsMultipleSharedBorrows)
{
    BorrowCell<int> value(42);
    const auto first = value.borrow();
    const auto second = value.borrow();

    EXPECT_EQ(*first, 42);
    EXPECT_EQ(*second, 42);
}

TEST(BorrowTest, RejectsMutableBorrowWhileSharedBorrowExists)
{
    BorrowCell<int> value(42);
    const auto shared = value.borrow();

    EXPECT_THROW(static_cast<void>(value.borrowMut()), diagnostics::Violation);
    EXPECT_EQ(*shared, 42);
}

TEST(BorrowTest, RejectsSharedBorrowWhileMutableBorrowExists)
{
    BorrowCell<int> value(42);
    auto exclusive = value.borrowMut();

    EXPECT_THROW(static_cast<void>(value.borrow()), diagnostics::Violation);
    EXPECT_EQ(*exclusive, 42);
}

TEST(BorrowTest, AllowsMutationAfterSharedBorrowEnds)
{
    BorrowCell<int> value(42);

    {
        const auto shared = value.borrow();
        EXPECT_EQ(*shared, 42);
    }

    auto mutableValue = value.borrowMut();
    *mutableValue = 7;
    EXPECT_EQ(*mutableValue, 7);
}

TEST(BorrowTest, TryBorrowReportsContention)
{
    BorrowCell<int> value(42);
    auto exclusive = value.borrowMut();
    const auto shared = value.tryBorrow();

    EXPECT_FALSE(shared);
    EXPECT_EQ(*exclusive, 42);
}

TEST(BorrowTest, TryBorrowMutReportsContention)
{
    BorrowCell<int> value(42);
    const auto shared = value.borrow();
    const auto exclusive = value.tryBorrowMut();

    EXPECT_FALSE(exclusive);
    EXPECT_EQ(*shared, 42);
}

TEST(BorrowTest, TryBorrowAllowsAccessWhenFree)
{
    BorrowCell<int> value(42);
    const auto shared = value.tryBorrow();

    EXPECT_TRUE(shared);
    EXPECT_EQ(*shared.value(), 42);
}

TEST(BorrowTest, TryBorrowMutAllowsAccessWhenFree)
{
    BorrowCell<int> value(42);
    auto exclusive = value.tryBorrowMut();

    EXPECT_TRUE(exclusive);
    *exclusive.value() = 7;
    EXPECT_EQ(*exclusive.value(), 7);
}

TEST(BorrowTest, MovesSharedBorrow)
{
    BorrowCell<int> value(42);
    auto first = value.borrow();
    const auto second = std::move(first);

    EXPECT_EQ(*second, 42);
    EXPECT_THROW(static_cast<void>(*first), diagnostics::Violation);
    EXPECT_THROW(static_cast<void>(value.borrowMut()), diagnostics::Violation);
}

TEST(BorrowTest, RejectsMovedFromMutableBorrow)
{
    BorrowCell<int> value(42);
    auto first = value.borrowMut();
    auto second = std::move(first);

    EXPECT_EQ(*second, 42);
    EXPECT_THROW(static_cast<void>(*first), diagnostics::Violation);
}

TEST(BorrowTest, ReleasesExclusiveBorrowAfterMovedHandleEnds)
{
    BorrowCell<int> value(42);

    {
        auto first = value.borrowMut();
        auto second = std::move(first);
        EXPECT_EQ(*second, 42);
    }

    auto exclusive = value.borrowMut();
    *exclusive = 7;
    EXPECT_EQ(*exclusive, 7);
}

TEST(BorrowTest, RejectsMovedFromCell)
{
    BorrowCell<int> value(42);
    auto moved = std::move(value);

    EXPECT_EQ(*moved.borrow(), 42);
    EXPECT_THROW(static_cast<void>(value.borrow()), diagnostics::Violation);
}

TEST(BorrowTest, TryBorrowSerializesConcurrentAccess)
{
    BorrowCell<int> value(0);
    constexpr int workerCount = 8;
    constexpr int attemptsPerWorker = 200;
    std::atomic<int> mutations{0};
    std::vector<std::thread> workers;
    workers.reserve(static_cast<std::size_t>(workerCount));

    for (int worker = 0; worker < workerCount; ++worker)
    {
        // clang-format off
        workers.emplace_back([&value, &mutations]()
        {
            for (int attempt = 0; attempt < attemptsPerWorker; ++attempt)
            {
                if (auto exclusive = value.tryBorrowMut())
                {
                    ++exclusive.value().get();
                    mutations.fetch_add(1, std::memory_order_relaxed);
                    continue;
                }

                if (auto shared = value.tryBorrow())
                {
                    static_cast<void>(shared.value().get());
                }
            }
        });
        // clang-format on
    }

    for (auto &worker : workers)
    {
        worker.join();
    }

    auto exclusive = value.borrowMut();
    EXPECT_EQ(*exclusive, mutations.load());
}
} // namespace safe::borrow::tests
