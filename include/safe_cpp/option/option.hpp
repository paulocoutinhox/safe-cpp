#pragma once

#include <optional>
#include <type_traits>
#include <utility>

namespace safe::option
{
namespace internal
{
[[noreturn]] void failEmptyOption();
}

template <typename T>
class Option final
{
public:
    Option() noexcept = default;
    Option(std::nullopt_t) noexcept
        : stored(std::nullopt)
    {
    }
    Option(const T &value)
        : stored(value)
    {
    }
    Option(T &&value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : stored(std::move(value))
    {
    }

    [[nodiscard]] static Option some(T value)
    {
        return Option(std::move(value));
    }

    [[nodiscard]] static Option none() noexcept
    {
        return Option();
    }

    [[nodiscard]] bool hasValue() const noexcept
    {
        return stored.has_value();
    }

    [[nodiscard]] explicit operator bool() const noexcept
    {
        return hasValue();
    }

    T &value() &
    {
        if (!stored)
        {
            internal::failEmptyOption();
        }

        return *stored;
    }

    const T &value() const &
    {
        if (!stored)
        {
            internal::failEmptyOption();
        }

        return *stored;
    }

    T &&value() &&
    {
        if (!stored)
        {
            internal::failEmptyOption();
        }

        return std::move(*stored);
    }

    template <typename U>
    [[nodiscard]] T valueOr(U &&fallback) const &
    {
        return stored.value_or(std::forward<U>(fallback));
    }

    template <typename F>
    [[nodiscard]] auto map(F &&function) const -> Option<std::decay_t<std::invoke_result_t<F, const T &>>>
    {
        using U = std::decay_t<std::invoke_result_t<F, const T &>>;
        if (!stored)
        {
            return Option<U>::none();
        }

        return Option<U>::some(std::forward<F>(function)(*stored));
    }

private:
    std::optional<T> stored;
};
} // namespace safe::option
