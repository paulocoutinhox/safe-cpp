#pragma once

#include <safe_cpp/option/option.hpp>

#include <cstddef>
#include <memory>
#include <mutex>
#include <utility>

namespace safe::borrow
{
namespace internal
{
[[noreturn]] void failSharedBorrow();
[[noreturn]] void failMutableBorrow();
[[noreturn]] void failMovedFrom();

template <typename T>
struct State final
{
    template <typename... Args>
    explicit State(Args &&...args)
        : value(std::forward<Args>(args)...)
    {
    }

    T value;
    std::mutex borrowMutex;
    std::size_t readers{0};
    bool writer{false};
};
} // namespace internal

template <typename T>
class Ref final
{
public:
    Ref(const Ref &) = delete;
    Ref &operator=(const Ref &) = delete;

    Ref(Ref &&other) noexcept
        : state(std::move(other.state))
    {
    }

    Ref &operator=(Ref &&other) noexcept
    {
        if (this != &other)
        {
            release();
            state = std::move(other.state);
        }

        return *this;
    }

    ~Ref() noexcept
    {
        release();
    }

    [[nodiscard]] const T &get() const { return live(); }
    [[nodiscard]] const T *operator->() const { return std::addressof(live()); }
    [[nodiscard]] const T &operator*() const { return live(); }

private:
    template <typename U>
    friend class BorrowCell;

    explicit Ref(std::shared_ptr<internal::State<T>> owned)
        : state(std::move(owned))
    {
    }

    const T &live() const
    {
        if (!state)
        {
            internal::failMovedFrom();
        }

        return state->value;
    }

    void release() noexcept
    {
        if (!state)
        {
            return;
        }

        auto held = state;
        {
            std::scoped_lock lock(held->borrowMutex);
            --held->readers;
        }

        state.reset();
    }

    std::shared_ptr<internal::State<T>> state;
};

template <typename T>
class Mut final
{
public:
    Mut(const Mut &) = delete;
    Mut &operator=(const Mut &) = delete;

    Mut(Mut &&other) noexcept
        : state(std::move(other.state))
    {
    }

    Mut &operator=(Mut &&other) noexcept
    {
        if (this != &other)
        {
            release();
            state = std::move(other.state);
        }

        return *this;
    }

    ~Mut() noexcept
    {
        release();
    }

    [[nodiscard]] T &get() { return live(); }
    [[nodiscard]] T *operator->() { return std::addressof(live()); }
    [[nodiscard]] T &operator*() { return live(); }

private:
    template <typename U>
    friend class BorrowCell;

    explicit Mut(std::shared_ptr<internal::State<T>> owned)
        : state(std::move(owned))
    {
    }

    T &live()
    {
        if (!state)
        {
            internal::failMovedFrom();
        }

        return state->value;
    }

    void release() noexcept
    {
        if (!state)
        {
            return;
        }

        auto held = state;
        {
            std::scoped_lock lock(held->borrowMutex);
            held->writer = false;
        }

        state.reset();
    }

    std::shared_ptr<internal::State<T>> state;
};

template <typename T>
class BorrowCell final
{
public:
    template <typename... Args>
    explicit BorrowCell(Args &&...args)
        : state(std::make_shared<internal::State<T>>(std::forward<Args>(args)...))
    {
    }

    BorrowCell(const BorrowCell &) = delete;
    BorrowCell &operator=(const BorrowCell &) = delete;
    BorrowCell(BorrowCell &&) noexcept = default;
    BorrowCell &operator=(BorrowCell &&) noexcept = default;

    [[nodiscard]] Ref<T> borrow() const
    {
        ensureLive();

        std::scoped_lock lock(state->borrowMutex);
        if (state->writer)
        {
            internal::failSharedBorrow();
        }

        ++state->readers;
        return Ref<T>(state);
    }

    [[nodiscard]] Mut<T> borrowMut()
    {
        ensureLive();

        std::scoped_lock lock(state->borrowMutex);
        if (state->writer || state->readers != 0)
        {
            internal::failMutableBorrow();
        }

        state->writer = true;
        return Mut<T>(state);
    }

    [[nodiscard]] option::Option<Ref<T>> tryBorrow() const
    {
        ensureLive();

        std::scoped_lock lock(state->borrowMutex);
        if (state->writer)
        {
            return option::Option<Ref<T>>::none();
        }

        ++state->readers;
        return option::Option<Ref<T>>::some(Ref<T>(state));
    }

    [[nodiscard]] option::Option<Mut<T>> tryBorrowMut()
    {
        ensureLive();

        std::scoped_lock lock(state->borrowMutex);
        if (state->writer || state->readers != 0)
        {
            return option::Option<Mut<T>>::none();
        }

        state->writer = true;
        return option::Option<Mut<T>>::some(Mut<T>(state));
    }

private:
    void ensureLive() const
    {
        if (!state)
        {
            internal::failMovedFrom();
        }
    }

    std::shared_ptr<internal::State<T>> state;
};
} // namespace safe::borrow
