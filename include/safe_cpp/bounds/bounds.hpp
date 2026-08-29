#pragma once

#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace safe::bounds
{
namespace internal
{
[[noreturn]] void failOutOfRange(std::size_t index, std::size_t size);
}

template <typename T, std::size_t N>
class Array final
{
public:
    Array() = default;
    explicit Array(std::array<T, N> items)
        : values(std::move(items))
    {
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return N; }

    T &at(std::size_t index)
    {
        if (index >= N)
        {
            internal::failOutOfRange(index, N);
        }

        return values[index];
    }

    const T &at(std::size_t index) const
    {
        if (index >= N)
        {
            internal::failOutOfRange(index, N);
        }

        return values[index];
    }

    T &operator[](std::size_t index) { return at(index); }
    const T &operator[](std::size_t index) const { return at(index); }

private:
    std::array<T, N> values{};
};

template <typename T>
class Vector final
{
public:
    Vector() = default;
    Vector(std::initializer_list<T> items)
        : values(items)
    {
    }

    [[nodiscard]] std::size_t size() const noexcept { return values.size(); }
    [[nodiscard]] bool empty() const noexcept { return values.empty(); }

    void reserve(std::size_t capacity) { values.reserve(capacity); }
    void push_back(T value) { values.push_back(std::move(value)); }

    template <typename... Args>
    T &emplace_back(Args &&...args)
    {
        return values.emplace_back(std::forward<Args>(args)...);
    }

    T &at(std::size_t index)
    {
        if (index >= values.size())
        {
            internal::failOutOfRange(index, values.size());
        }

        return values[index];
    }

    const T &at(std::size_t index) const
    {
        if (index >= values.size())
        {
            internal::failOutOfRange(index, values.size());
        }

        return values[index];
    }

    T &operator[](std::size_t index) { return at(index); }
    const T &operator[](std::size_t index) const { return at(index); }

private:
    std::vector<T> values;
};

template <typename T>
class Slice final
{
public:
    explicit Slice(std::span<T> items) noexcept
        : values(items)
    {
    }

    template <std::size_t N>
    explicit Slice(T (&items)[N]) noexcept
        : values(items, N)
    {
    }

    [[nodiscard]] std::size_t size() const noexcept { return values.size(); }
    [[nodiscard]] bool empty() const noexcept { return values.empty(); }

    T &at(std::size_t index)
    {
        if (index >= values.size())
        {
            internal::failOutOfRange(index, values.size());
        }

        return values[index];
    }

    const T &at(std::size_t index) const
    {
        if (index >= values.size())
        {
            internal::failOutOfRange(index, values.size());
        }

        return values[index];
    }

    T &operator[](std::size_t index) { return at(index); }
    const T &operator[](std::size_t index) const { return at(index); }

    [[nodiscard]] Slice subspan(std::size_t offset, std::size_t count) const
    {
        if (offset > values.size())
        {
            internal::failOutOfRange(offset, values.size());
        }

        if (count > values.size() - offset)
        {
            internal::failOutOfRange(offset, values.size());
        }

        return Slice(values.subspan(offset, count));
    }

private:
    std::span<T> values;
};

class String final
{
public:
    String() = default;
    explicit String(std::string value)
        : stored(std::move(value))
    {
    }
    explicit String(std::string_view value)
        : stored(value)
    {
    }

    [[nodiscard]] std::size_t size() const noexcept { return stored.size(); }
    [[nodiscard]] bool empty() const noexcept { return stored.empty(); }
    [[nodiscard]] std::string_view view() const noexcept { return stored; }

    char &at(std::size_t index);
    const char &at(std::size_t index) const;
    char &operator[](std::size_t index) { return at(index); }
    const char &operator[](std::size_t index) const { return at(index); }

private:
    std::string stored;
};
} // namespace safe::bounds
