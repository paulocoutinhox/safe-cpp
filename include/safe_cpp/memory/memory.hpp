#pragma once

#include <safe_cpp/option/option.hpp>

#include <memory>
#include <type_traits>
#include <utility>

namespace safe::memory
{
namespace internal
{
[[noreturn]] void failNullPointer();
[[noreturn]] void failMovedFrom();
} // namespace internal

template <typename T>
class NonNull final
{
public:
    explicit NonNull(T &value) noexcept
        : pointer(std::addressof(value))
    {
    }

    explicit NonNull(T *source)
        : pointer(source)
    {
        if (pointer == nullptr)
        {
            internal::failNullPointer();
        }
    }

    [[nodiscard]] static option::Option<NonNull> from(T *pointer)
    {
        if (pointer == nullptr)
        {
            return option::Option<NonNull>::none();
        }

        return option::Option<NonNull>::some(NonNull(*pointer));
    }

    [[nodiscard]] T &get() const noexcept { return *pointer; }
    [[nodiscard]] T *operator->() const noexcept { return pointer; }
    [[nodiscard]] T &operator*() const noexcept { return *pointer; }

private:
    T *pointer;
};

template <typename T>
class Box final
{
public:
    Box(const Box &) = delete;
    Box &operator=(const Box &) = delete;
    Box(Box &&) noexcept = default;
    Box &operator=(Box &&) noexcept = default;

    template <typename... Args>
    [[nodiscard]] static Box make(Args &&...args)
    {
        return Box(std::make_unique<T>(std::forward<Args>(args)...));
    }

    [[nodiscard]] T &get() { return live(); }
    [[nodiscard]] const T &get() const { return live(); }
    [[nodiscard]] T *operator->() { return std::addressof(live()); }
    [[nodiscard]] const T *operator->() const { return std::addressof(live()); }
    [[nodiscard]] T &operator*() { return live(); }
    [[nodiscard]] const T &operator*() const { return live(); }

    [[nodiscard]] std::unique_ptr<T> intoUnique() &&
    {
        return std::move(livePointer());
    }

private:
    explicit Box(std::unique_ptr<T> owned)
        : pointer(std::move(owned))
    {
        if (!pointer)
        {
            internal::failNullPointer();
        }
    }

    std::unique_ptr<T> &livePointer()
    {
        if (!pointer)
        {
            internal::failMovedFrom();
        }

        return pointer;
    }

    T &live() { return *livePointer(); }
    const T &live() const
    {
        if (!pointer)
        {
            internal::failMovedFrom();
        }

        return *pointer;
    }

    std::unique_ptr<T> pointer;
};

template <typename T>
class Weak;

template <typename T>
class Arc final
{
public:
    Arc(const Arc &) noexcept = default;
    Arc &operator=(const Arc &) noexcept = default;
    Arc(Arc &&) noexcept = default;
    Arc &operator=(Arc &&) noexcept = default;

    template <typename... Args>
    [[nodiscard]] static Arc make(Args &&...args)
    {
        return Arc(std::make_shared<T>(std::forward<Args>(args)...));
    }

    [[nodiscard]] static Arc fromShared(std::shared_ptr<T> pointer)
    {
        return Arc(std::move(pointer));
    }

    [[nodiscard]] const T &get() const { return live(); }
    [[nodiscard]] const T *operator->() const { return std::addressof(live()); }
    [[nodiscard]] const T &operator*() const { return live(); }
    [[nodiscard]] long use_count() const noexcept { return pointer.use_count(); }
    [[nodiscard]] Weak<T> downgrade() const { return Weak<T>(livePointer()); }

private:
    friend class Weak<T>;

    explicit Arc(std::shared_ptr<T> owned)
        : pointer(std::move(owned))
    {
        if (!pointer)
        {
            internal::failNullPointer();
        }
    }

    const std::shared_ptr<T> &livePointer() const
    {
        if (!pointer)
        {
            internal::failMovedFrom();
        }

        return pointer;
    }

    const T &live() const { return *livePointer(); }

    std::shared_ptr<T> pointer;
};

template <typename T>
class Weak final
{
public:
    Weak() noexcept = default;

    [[nodiscard]] option::Option<Arc<T>> upgrade() const
    {
        auto locked = pointer.lock();
        if (!locked)
        {
            return option::Option<Arc<T>>::none();
        }

        return option::Option<Arc<T>>::some(Arc<T>::fromShared(std::move(locked)));
    }

    [[nodiscard]] bool expired() const noexcept
    {
        return pointer.expired();
    }

private:
    friend class Arc<T>;

    explicit Weak(const std::shared_ptr<T> &owned) noexcept
        : pointer(owned)
    {
    }

    std::weak_ptr<T> pointer;
};
} // namespace safe::memory
