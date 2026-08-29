#pragma once

#include <concepts>
#include <limits>
#include <type_traits>

namespace safe::arithmetic
{
namespace internal
{
[[noreturn]] void failOverflow();
[[noreturn]] void failDivisionByZero();
} // namespace internal

template <std::integral T>
class CheckedInteger final
{
    static_assert(!std::is_same_v<T, bool>, "CheckedInteger requires a non-bool integral type");

public:
    constexpr CheckedInteger() noexcept = default;
    constexpr explicit CheckedInteger(T value) noexcept
        : stored(value)
    {
    }

    [[nodiscard]] constexpr T value() const noexcept { return stored; }

    [[nodiscard]] CheckedInteger operator+(CheckedInteger other) const
    {
        T result{};
        if (addOverflow(stored, other.stored, result))
        {
            internal::failOverflow();
        }

        return CheckedInteger(result);
    }

    [[nodiscard]] CheckedInteger operator-(CheckedInteger other) const
    {
        T result{};
        if (subtractOverflow(stored, other.stored, result))
        {
            internal::failOverflow();
        }

        return CheckedInteger(result);
    }

    [[nodiscard]] CheckedInteger operator*(CheckedInteger other) const
    {
        T result{};
        if (multiplyOverflow(stored, other.stored, result))
        {
            internal::failOverflow();
        }

        return CheckedInteger(result);
    }

    [[nodiscard]] CheckedInteger operator/(CheckedInteger other) const
    {
        if (other.stored == 0)
        {
            internal::failDivisionByZero();
        }

        if constexpr (std::is_signed_v<T>)
        {
            if (stored == std::numeric_limits<T>::min() && other.stored == T{-1})
            {
                internal::failOverflow();
            }
        }

        return CheckedInteger(static_cast<T>(stored / other.stored));
    }

    [[nodiscard]] CheckedInteger operator%(CheckedInteger other) const
    {
        if (other.stored == 0)
        {
            internal::failDivisionByZero();
        }

        if constexpr (std::is_signed_v<T>)
        {
            if (stored == std::numeric_limits<T>::min() && other.stored == T{-1})
            {
                internal::failOverflow();
            }
        }

        return CheckedInteger(static_cast<T>(stored % other.stored));
    }

    [[nodiscard]] CheckedInteger operator-() const
    {
        if constexpr (std::is_signed_v<T>)
        {
            if (stored == std::numeric_limits<T>::min())
            {
                internal::failOverflow();
            }
        }
        else if (stored != 0)
        {
            internal::failOverflow();
        }

        return CheckedInteger(static_cast<T>(-stored));
    }

private:
    static bool addOverflow(T left, T right, T &result) noexcept
    {
#if defined(__GNUC__) || defined(__clang__)
        return __builtin_add_overflow(left, right, &result);
#else
        if constexpr (std::is_unsigned_v<T>)
        {
            result = static_cast<T>(left + right);
            return result < left;
        }
        else
        {
            if ((right > 0 && left > std::numeric_limits<T>::max() - right) ||
                (right < 0 && left < std::numeric_limits<T>::min() - right))
            {
                return true;
            }

            result = static_cast<T>(left + right);
            return false;
        }
#endif
    }

    static bool subtractOverflow(T left, T right, T &result) noexcept
    {
#if defined(__GNUC__) || defined(__clang__)
        return __builtin_sub_overflow(left, right, &result);
#else
        if constexpr (std::is_unsigned_v<T>)
        {
            if (left < right)
            {
                return true;
            }

            result = static_cast<T>(left - right);
            return false;
        }
        else
        {
            if ((right > 0 && left < std::numeric_limits<T>::min() + right) ||
                (right < 0 && left > std::numeric_limits<T>::max() + right))
            {
                return true;
            }

            result = static_cast<T>(left - right);
            return false;
        }
#endif
    }

    static bool multiplyOverflow(T left, T right, T &result) noexcept
    {
#if defined(__GNUC__) || defined(__clang__)
        return __builtin_mul_overflow(left, right, &result);
#else
        if (left == 0 || right == 0)
        {
            result = 0;
            return false;
        }

        if constexpr (std::is_unsigned_v<T>)
        {
            if (left > std::numeric_limits<T>::max() / right)
            {
                return true;
            }
        }
        else
        {
            if (left == -1 && right == std::numeric_limits<T>::min())
            {
                return true;
            }
            if (right == -1 && left == std::numeric_limits<T>::min())
            {
                return true;
            }
            if (left > 0)
            {
                if ((right > 0 && left > std::numeric_limits<T>::max() / right) ||
                    (right < 0 && right < std::numeric_limits<T>::min() / left))
                {
                    return true;
                }
            }
            else if ((right > 0 && left < std::numeric_limits<T>::min() / right) ||
                     (right < 0 && left < std::numeric_limits<T>::max() / right))
            {
                return true;
            }
        }

        result = static_cast<T>(left * right);
        return false;
#endif
    }

    T stored{};
};
} // namespace safe::arithmetic
