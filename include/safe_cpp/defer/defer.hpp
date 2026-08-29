#pragma once

#include <exception>
#include <type_traits>
#include <utility>

namespace safe::scope
{
namespace internal
{
[[noreturn]] void terminateFromDeferredAction() noexcept;
}

template <typename F>
class DeferFinalizer final
{
public:
    explicit DeferFinalizer(F &&action) noexcept(std::is_nothrow_move_constructible_v<F>)
        : function(std::move(action))
    {
    }

    DeferFinalizer(const DeferFinalizer &) = delete;
    DeferFinalizer &operator=(const DeferFinalizer &) = delete;

    DeferFinalizer(DeferFinalizer &&other) noexcept(std::is_nothrow_move_constructible_v<F>)
        : function(std::move(other.function))
        , armed(other.armed)
    {
        other.armed = false;
    }

    DeferFinalizer &operator=(DeferFinalizer &&) = delete;

    ~DeferFinalizer() noexcept
    {
        if (!armed)
        {
            return;
        }

        try
        {
            function();
        }
        catch (...)
        {
            internal::terminateFromDeferredAction();
        }
    }

    void cancel() noexcept
    {
        armed = false;
    }

    [[nodiscard]] bool active() const noexcept
    {
        return armed;
    }

private:
    F function;
    bool armed{true};
};

struct Deferrer final
{
    template <typename F>
    [[nodiscard]] auto operator<<(F &&function) const
    {
        using Function = std::decay_t<F>;
        return DeferFinalizer<Function>{Function{std::forward<F>(function)}};
    }
};

inline constexpr Deferrer deferrer{};

template <typename F>
[[nodiscard]] auto makeDefer(F &&function)
{
    return deferrer << std::forward<F>(function);
}
} // namespace safe::scope

#define SAFE_CPP_TOKEN_PASTE_IMPL(x, y) x##y
#define SAFE_CPP_TOKEN_PASTE(x, y) SAFE_CPP_TOKEN_PASTE_IMPL(x, y)
#define SAFE_CPP_DEFER \
    [[maybe_unused]] auto SAFE_CPP_TOKEN_PASTE(safeCppDeferredAction, __LINE__) = ::safe::scope::deferrer << [&]()
#define defer SAFE_CPP_DEFER
