#pragma once

#include <functional>
#include <mutex>
#include <type_traits>
#include <utility>

namespace safe::concurrency
{
template <typename T>
class Mutex final
{
public:
    template <typename... Args>
    explicit Mutex(Args &&...args)
        : stored(std::forward<Args>(args)...)
    {
    }

    Mutex(const Mutex &) = delete;
    Mutex &operator=(const Mutex &) = delete;
    Mutex(Mutex &&) = delete;
    Mutex &operator=(Mutex &&) = delete;

    template <typename F>
    decltype(auto) withLock(F &&function)
    {
        using Return = std::invoke_result_t<F, T &>;
        static_assert(!std::is_reference_v<Return>, "Mutex callbacks cannot return references to protected data");
        static_assert(!std::is_pointer_v<Return>, "Mutex callbacks cannot return pointers to protected data");

        std::scoped_lock lock(mutex);
        return std::invoke(std::forward<F>(function), stored);
    }

    template <typename F>
    decltype(auto) withLock(F &&function) const
    {
        using Return = std::invoke_result_t<F, const T &>;
        static_assert(!std::is_reference_v<Return>, "Mutex callbacks cannot return references to protected data");
        static_assert(!std::is_pointer_v<Return>, "Mutex callbacks cannot return pointers to protected data");

        std::scoped_lock lock(mutex);
        return std::invoke(std::forward<F>(function), stored);
    }

private:
    mutable std::mutex mutex;
    T stored;
};
} // namespace safe::concurrency
