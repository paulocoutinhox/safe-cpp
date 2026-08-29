#pragma once

#include <safe_cpp/config/config.hpp>

#include <functional>
#include <type_traits>
#include <utility>
#if !SAFE_CPP_HAS_STD_EXPECTED
#include <variant>
#endif

namespace safe::result
{
namespace internal
{
[[noreturn]] void failMissingValue();
[[noreturn]] void failMissingError();

#if !SAFE_CPP_HAS_STD_EXPECTED
template <typename T>
struct ValueStorage final
{
    T value;
};

template <typename E>
struct ErrorStorage final
{
    E error;
};
#endif
} // namespace internal

template <typename E>
class Unexpected final
{
public:
    explicit Unexpected(E error)
        : storedError(std::move(error))
    {
    }

    const E &error() const & noexcept { return storedError; }
    E &&error() && noexcept { return std::move(storedError); }

private:
    E storedError;
};

template <typename E>
[[nodiscard]] auto unexpected(E error)
{
    return Unexpected<E>{std::move(error)};
}

template <typename T, typename E>
class Result final
{
public:
    Result(const T &value)
        : storage(makeValue(value))
    {
    }
    Result(T &&value)
        : storage(makeValue(std::move(value)))
    {
    }
    Result(Unexpected<E> error)
        : storage(makeError(std::move(error).error()))
    {
    }

    [[nodiscard]] static Result ok(T value)
    {
        return Result(std::move(value));
    }

    [[nodiscard]] static Result err(E error)
    {
        return Result(unexpected(std::move(error)));
    }

    [[nodiscard]] bool hasValue() const noexcept
    {
#if SAFE_CPP_HAS_STD_EXPECTED
        return storage.has_value();
#else
        return std::holds_alternative<internal::ValueStorage<T>>(storage);
#endif
    }

    [[nodiscard]] explicit operator bool() const noexcept
    {
        return hasValue();
    }

    T &value() &
    {
        if (!hasValue())
        {
            internal::failMissingValue();
        }

#if SAFE_CPP_HAS_STD_EXPECTED
        return storage.value();
#else
        return std::get<internal::ValueStorage<T>>(storage).value;
#endif
    }

    const T &value() const &
    {
        if (!hasValue())
        {
            internal::failMissingValue();
        }

#if SAFE_CPP_HAS_STD_EXPECTED
        return storage.value();
#else
        return std::get<internal::ValueStorage<T>>(storage).value;
#endif
    }

    T &&value() &&
    {
        if (!hasValue())
        {
            internal::failMissingValue();
        }

#if SAFE_CPP_HAS_STD_EXPECTED
        return std::move(storage).value();
#else
        return std::move(std::get<internal::ValueStorage<T>>(storage).value);
#endif
    }

    E &error() &
    {
        if (hasValue())
        {
            internal::failMissingError();
        }

#if SAFE_CPP_HAS_STD_EXPECTED
        return storage.error();
#else
        return std::get<internal::ErrorStorage<E>>(storage).error;
#endif
    }

    const E &error() const &
    {
        if (hasValue())
        {
            internal::failMissingError();
        }

#if SAFE_CPP_HAS_STD_EXPECTED
        return storage.error();
#else
        return std::get<internal::ErrorStorage<E>>(storage).error;
#endif
    }

    E &&error() &&
    {
        if (hasValue())
        {
            internal::failMissingError();
        }

#if SAFE_CPP_HAS_STD_EXPECTED
        return std::move(storage).error();
#else
        return std::move(std::get<internal::ErrorStorage<E>>(storage).error);
#endif
    }

    template <typename F>
    [[nodiscard]] auto map(F &&function) const -> Result<std::decay_t<std::invoke_result_t<F, const T &>>, E>
    {
        using U = std::decay_t<std::invoke_result_t<F, const T &>>;
        if (!hasValue())
        {
            return Result<U, E>::err(error());
        }

        if constexpr (std::is_void_v<U>)
        {
            std::forward<F>(function)(value());
            return Result<U, E>::ok();
        }
        else
        {
            return Result<U, E>::ok(std::forward<F>(function)(value()));
        }
    }

private:
#if SAFE_CPP_HAS_STD_EXPECTED
    using Storage = std::expected<T, E>;

    template <typename U>
    static Storage makeValue(U &&value)
    {
        return Storage{std::in_place, std::forward<U>(value)};
    }

    static Storage makeError(E error)
    {
        return std::unexpected<E>{std::move(error)};
    }
#else
    using Storage = std::variant<internal::ValueStorage<T>, internal::ErrorStorage<E>>;

    template <typename U>
    static Storage makeValue(U &&value)
    {
        return internal::ValueStorage<T>{std::forward<U>(value)};
    }

    static Storage makeError(E error)
    {
        return internal::ErrorStorage<E>{std::move(error)};
    }
#endif

    Storage storage;
};

template <typename E>
class Result<void, E> final
{
public:
    Result()
        : storage(makeValue())
    {
    }
    Result(Unexpected<E> error)
        : storage(makeError(std::move(error).error()))
    {
    }

    [[nodiscard]] static Result ok()
    {
        return Result();
    }

    [[nodiscard]] static Result err(E error)
    {
        return Result(unexpected(std::move(error)));
    }

    [[nodiscard]] bool hasValue() const noexcept
    {
#if SAFE_CPP_HAS_STD_EXPECTED
        return storage.has_value();
#else
        return std::holds_alternative<std::monostate>(storage);
#endif
    }

    [[nodiscard]] explicit operator bool() const noexcept
    {
        return hasValue();
    }

    void value() const
    {
        if (!hasValue())
        {
            internal::failMissingValue();
        }
    }

    E &error() &
    {
        if (hasValue())
        {
            internal::failMissingError();
        }

#if SAFE_CPP_HAS_STD_EXPECTED
        return storage.error();
#else
        return std::get<internal::ErrorStorage<E>>(storage).error;
#endif
    }

    const E &error() const &
    {
        if (hasValue())
        {
            internal::failMissingError();
        }

#if SAFE_CPP_HAS_STD_EXPECTED
        return storage.error();
#else
        return std::get<internal::ErrorStorage<E>>(storage).error;
#endif
    }

    E &&error() &&
    {
        if (hasValue())
        {
            internal::failMissingError();
        }

#if SAFE_CPP_HAS_STD_EXPECTED
        return std::move(storage).error();
#else
        return std::move(std::get<internal::ErrorStorage<E>>(storage).error);
#endif
    }

    template <typename F>
    [[nodiscard]] auto map(F &&function) const -> Result<std::decay_t<std::invoke_result_t<F>>, E>
    {
        using U = std::decay_t<std::invoke_result_t<F>>;
        if (!hasValue())
        {
            return Result<U, E>::err(error());
        }

        if constexpr (std::is_void_v<U>)
        {
            std::forward<F>(function)();
            return Result<U, E>::ok();
        }
        else
        {
            return Result<U, E>::ok(std::forward<F>(function)());
        }
    }

private:
#if SAFE_CPP_HAS_STD_EXPECTED
    using Storage = std::expected<void, E>;

    static Storage makeValue()
    {
        return Storage{};
    }

    static Storage makeError(E error)
    {
        return std::unexpected<E>{std::move(error)};
    }
#else
    using Storage = std::variant<std::monostate, internal::ErrorStorage<E>>;

    static Storage makeValue()
    {
        return std::monostate{};
    }

    static Storage makeError(E error)
    {
        return internal::ErrorStorage<E>{std::move(error)};
    }
#endif

    Storage storage;
};
} // namespace safe::result

#define SAFE_CPP_RESULT_TOKEN_PASTE_IMPL(x, y) x##y
#define SAFE_CPP_RESULT_TOKEN_PASTE(x, y) SAFE_CPP_RESULT_TOKEN_PASTE_IMPL(x, y)
#define SAFE_CPP_TRY(name, expression)                                                                              \
    auto SAFE_CPP_RESULT_TOKEN_PASTE(safeCppResult, __LINE__) = (expression);                                       \
    if (!SAFE_CPP_RESULT_TOKEN_PASTE(safeCppResult, __LINE__))                                                      \
    {                                                                                                               \
        return ::safe::result::unexpected(std::move(SAFE_CPP_RESULT_TOKEN_PASTE(safeCppResult, __LINE__)).error()); \
    }                                                                                                               \
    auto name = std::move(SAFE_CPP_RESULT_TOKEN_PASTE(safeCppResult, __LINE__)).value()

#define SAFE_CPP_TRY_VOID(expression)                                                                               \
    auto SAFE_CPP_RESULT_TOKEN_PASTE(safeCppResult, __LINE__) = (expression);                                       \
    if (!SAFE_CPP_RESULT_TOKEN_PASTE(safeCppResult, __LINE__))                                                      \
    {                                                                                                               \
        return ::safe::result::unexpected(std::move(SAFE_CPP_RESULT_TOKEN_PASTE(safeCppResult, __LINE__)).error()); \
    }                                                                                                               \
    SAFE_CPP_RESULT_TOKEN_PASTE(safeCppResult, __LINE__).value()
