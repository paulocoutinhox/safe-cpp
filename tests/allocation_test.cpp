#include <safe_cpp/allocation/allocation.hpp>
#include <safe_cpp/diagnostics/diagnostics.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace safe::allocation::tests
{
TEST(AllocationTest, HeapAllocatorOwnsTypedObjects)
{
    auto allocator = HeapAllocator::create();
    auto value = allocator->make<std::string>("safe");
    EXPECT_EQ(*value, "safe");
}

TEST(AllocationTest, ArenaAllocatorKeepsObjectsAlive)
{
    auto allocator = ArenaAllocator::create(1024);
    auto first = allocator->make<int>(20);
    auto second = allocator->make<int>(22);
    EXPECT_EQ(*first + *second, 42);
}

TEST(AllocationTest, MovesHeapAllocation)
{
    auto allocator = HeapAllocator::create();
    auto first = allocator->make<int>(42);
    auto second = std::move(first);
    EXPECT_EQ(*second, 42);
    EXPECT_THROW(static_cast<void>(*first), diagnostics::Violation);
}

TEST(AllocationTest, ReleasesMemoryWhenConstructionThrows)
{
    struct Throws
    {
        Throws()
        {
            throw std::runtime_error("Construction failed");
        }
    };

    auto allocator = HeapAllocator::create();
    EXPECT_THROW(static_cast<void>(allocator->make<Throws>()), std::runtime_error);
}

TEST(AllocationTest, RejectsNullAllocation)
{
    class NullAllocator final : public Allocator
    {
    public:
        NullAllocator() = default;

    private:
        void *allocateBytes(std::size_t, std::size_t) override
        {
            return nullptr;
        }

        void deallocateBytes(void *, std::size_t, std::size_t) noexcept override
        {
        }
    };

    auto allocator = std::make_shared<NullAllocator>();
    EXPECT_THROW(static_cast<void>(allocator->make<int>(1)), diagnostics::Violation);
}

TEST(AllocationTest, RejectsUnsharedAllocator)
{
    class StackAllocator final : public Allocator
    {
    public:
        StackAllocator() = default;

    private:
        void *allocateBytes(std::size_t size, std::size_t alignment) override
        {
            return ::operator new(size, std::align_val_t(alignment));
        }

        void deallocateBytes(void *pointer, std::size_t, std::size_t alignment) noexcept override
        {
            ::operator delete(pointer, std::align_val_t(alignment));
        }
    };

    StackAllocator allocator;
    EXPECT_THROW(static_cast<void>(allocator.make<int>(1)), diagnostics::Violation);
}

TEST(AllocationTest, HeapAllocatorHonorsOverAlignment)
{
    struct alignas(64) CacheLine
    {
        int value;

        explicit CacheLine(int number)
            : value(number)
        {
        }
    };

    auto allocator = HeapAllocator::create();
    auto value = allocator->make<CacheLine>(42);

    EXPECT_EQ(value->value, 42);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(std::addressof(*value)) % 64u, 0u);
}

TEST(AllocationTest, HeapAllocatorServesConcurrentAllocations)
{
    auto allocator = HeapAllocator::create();
    constexpr int workerCount = 4;
    constexpr int allocationsPerWorker = 32;
    std::vector<std::vector<int>> snapshots(static_cast<std::size_t>(workerCount));
    std::vector<std::thread> workers;
    workers.reserve(static_cast<std::size_t>(workerCount));

    for (int worker = 0; worker < workerCount; ++worker)
    {
        // clang-format off
        workers.emplace_back([allocator, &snapshots, worker]()
        {
            std::vector<Allocation<int>> locals;
            locals.reserve(static_cast<std::size_t>(allocationsPerWorker));

            for (int index = 0; index < allocationsPerWorker; ++index)
            {
                locals.push_back(allocator->make<int>((worker * allocationsPerWorker) + index));
            }

            auto& snapshot = snapshots[static_cast<std::size_t>(worker)];
            snapshot.reserve(locals.size());

            for (const auto& allocation : locals)
            {
                snapshot.push_back(*allocation);
            }
        });
        // clang-format on
    }

    for (auto &worker : workers)
    {
        worker.join();
    }

    for (int worker = 0; worker < workerCount; ++worker)
    {
        const auto &snapshot = snapshots[static_cast<std::size_t>(worker)];
        ASSERT_EQ(snapshot.size(), static_cast<std::size_t>(allocationsPerWorker));

        for (int index = 0; index < allocationsPerWorker; ++index)
        {
            EXPECT_EQ(snapshot[static_cast<std::size_t>(index)], (worker * allocationsPerWorker) + index);
        }
    }
}

TEST(AllocationTest, ArenaAllocatorServesConcurrentAllocations)
{
    auto allocator = ArenaAllocator::create(64 * 1024);
    constexpr int workerCount = 4;
    constexpr int allocationsPerWorker = 32;
    std::vector<std::vector<int>> snapshots(static_cast<std::size_t>(workerCount));
    std::vector<std::thread> workers;
    workers.reserve(static_cast<std::size_t>(workerCount));

    for (int worker = 0; worker < workerCount; ++worker)
    {
        // clang-format off
        workers.emplace_back([allocator, &snapshots, worker]()
        {
            std::vector<Allocation<int>> locals;
            locals.reserve(static_cast<std::size_t>(allocationsPerWorker));

            for (int index = 0; index < allocationsPerWorker; ++index)
            {
                locals.push_back(allocator->make<int>((worker * allocationsPerWorker) + index));
            }

            auto& snapshot = snapshots[static_cast<std::size_t>(worker)];
            snapshot.reserve(locals.size());

            for (const auto& allocation : locals)
            {
                snapshot.push_back(*allocation);
            }
        });
        // clang-format on
    }

    for (auto &worker : workers)
    {
        worker.join();
    }

    for (int worker = 0; worker < workerCount; ++worker)
    {
        const auto &snapshot = snapshots[static_cast<std::size_t>(worker)];
        ASSERT_EQ(snapshot.size(), static_cast<std::size_t>(allocationsPerWorker));

        for (int index = 0; index < allocationsPerWorker; ++index)
        {
            EXPECT_EQ(snapshot[static_cast<std::size_t>(index)], (worker * allocationsPerWorker) + index);
        }
    }
}
} // namespace safe::allocation::tests
