#pragma once

#include <cstddef>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <new>
#include <type_traits>
#include <utility>

namespace safe::allocation
{
namespace internal
{
[[noreturn]] void failMovedFrom();
[[noreturn]] void failUnsharedAllocator();
[[noreturn]] void failNullAllocation();
} // namespace internal

class Allocator;

template <typename T>
class Allocation final
{
public:
    Allocation(const Allocation &) = delete;
    Allocation &operator=(const Allocation &) = delete;

    Allocation(Allocation &&other) noexcept
        : allocator(std::move(other.allocator))
        , pointer(std::exchange(other.pointer, nullptr))
    {
    }

    Allocation &operator=(Allocation &&other) noexcept
    {
        if (this != &other)
        {
            reset();
            allocator = std::move(other.allocator);
            pointer = std::exchange(other.pointer, nullptr);
        }

        return *this;
    }

    ~Allocation() noexcept
    {
        reset();
    }

    [[nodiscard]] T &get() { return live(); }
    [[nodiscard]] const T &get() const { return live(); }
    [[nodiscard]] T *operator->() { return std::addressof(live()); }
    [[nodiscard]] const T *operator->() const { return std::addressof(live()); }
    [[nodiscard]] T &operator*() { return live(); }
    [[nodiscard]] const T &operator*() const { return live(); }

private:
    friend class Allocator;

    T &live()
    {
        if (pointer == nullptr)
        {
            internal::failMovedFrom();
        }

        return *pointer;
    }

    const T &live() const
    {
        if (pointer == nullptr)
        {
            internal::failMovedFrom();
        }

        return *pointer;
    }

    Allocation(std::shared_ptr<Allocator> owned, T *address) noexcept
        : allocator(std::move(owned))
        , pointer(address)
    {
    }

    void reset() noexcept;

    std::shared_ptr<Allocator> allocator;
    T *pointer{nullptr};
};

class Allocator : public std::enable_shared_from_this<Allocator>
{
public:
    virtual ~Allocator() = default;

    Allocator(const Allocator &) = delete;
    Allocator &operator=(const Allocator &) = delete;
    Allocator(Allocator &&) = delete;
    Allocator &operator=(Allocator &&) = delete;

    template <typename T, typename... Args>
    [[nodiscard]] Allocation<T> make(Args &&...args)
    {
        if (weak_from_this().expired())
        {
            internal::failUnsharedAllocator();
        }

        void *memory = allocateBytes(sizeof(T), alignof(T));
        if (memory == nullptr)
        {
            internal::failNullAllocation();
        }

        try
        {
            T *value = std::construct_at(static_cast<T *>(memory), std::forward<Args>(args)...);
            return Allocation<T>(shared_from_this(), value);
        }
        catch (...)
        {
            deallocateBytes(memory, sizeof(T), alignof(T));
            throw;
        }
    }

protected:
    Allocator() = default;

private:
    template <typename T>
    friend class Allocation;

    virtual void *allocateBytes(std::size_t size, std::size_t alignment) = 0;
    virtual void deallocateBytes(void *pointer, std::size_t size, std::size_t alignment) noexcept = 0;
};

class HeapAllocator final : public Allocator
{
public:
    [[nodiscard]] static std::shared_ptr<HeapAllocator> create();

private:
    HeapAllocator() = default;

    void *allocateBytes(std::size_t size, std::size_t alignment) override;
    void deallocateBytes(void *pointer, std::size_t size, std::size_t alignment) noexcept override;
};

class ArenaAllocator final : public Allocator
{
public:
    [[nodiscard]] static std::shared_ptr<ArenaAllocator> create(std::size_t initialSize = 64 * 1024);

private:
    explicit ArenaAllocator(std::size_t initialSize);

    void *allocateBytes(std::size_t size, std::size_t alignment) override;
    void deallocateBytes(void *pointer, std::size_t size, std::size_t alignment) noexcept override;

    std::unique_ptr<std::byte[]> initialBuffer;
    std::pmr::monotonic_buffer_resource resource;
    std::mutex mutex;
};

template <typename T>
void Allocation<T>::reset() noexcept
{
    if (pointer == nullptr)
    {
        return;
    }

    std::destroy_at(pointer);
    allocator->deallocateBytes(pointer, sizeof(T), alignof(T));
    pointer = nullptr;
    allocator.reset();
}
} // namespace safe::allocation
