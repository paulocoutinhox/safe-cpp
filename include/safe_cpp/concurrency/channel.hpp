#pragma once

#include <safe_cpp/option/option.hpp>
#include <safe_cpp/result/result.hpp>

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

namespace safe::concurrency
{
enum class ChannelError
{
    closed
};

[[nodiscard]] std::string_view channelErrorMessage(ChannelError error) noexcept;

namespace internal
{
[[noreturn]] void failMovedFrom();

template <typename T>
struct ChannelState final
{
    std::mutex mutex;
    std::condition_variable condition;
    std::deque<T> queue;
    bool closed{false};
};
} // namespace internal

template <typename T>
class Sender final
{
public:
    Sender() = delete;

    [[nodiscard]] result::Result<void, ChannelError> send(T value) const
    {
        ensureLive();

        {
            std::scoped_lock lock(state->mutex);
            if (state->closed)
            {
                return result::Result<void, ChannelError>::err(ChannelError::closed);
            }

            state->queue.push_back(std::move(value));
        }

        state->condition.notify_one();
        return result::Result<void, ChannelError>::ok();
    }

    void close() const
    {
        ensureLive();

        {
            std::scoped_lock lock(state->mutex);
            state->closed = true;
        }

        state->condition.notify_all();
    }

    [[nodiscard]] bool isClosed() const
    {
        ensureLive();

        std::scoped_lock lock(state->mutex);
        return state->closed;
    }

private:
    template <typename U>
    friend class Channel;

    explicit Sender(std::shared_ptr<internal::ChannelState<T>> owned)
        : state(std::move(owned))
    {
    }

    void ensureLive() const
    {
        if (!state)
        {
            internal::failMovedFrom();
        }
    }

    std::shared_ptr<internal::ChannelState<T>> state;
};

template <typename T>
class Receiver final
{
public:
    Receiver() = delete;

    [[nodiscard]] option::Option<T> receive() const
    {
        ensureLive();

        std::unique_lock lock(state->mutex);
        // clang-format off
        state->condition.wait(lock, [this]()
        {
            return state->closed || !state->queue.empty();
        });
        // clang-format on

        if (state->queue.empty())
        {
            return option::Option<T>::none();
        }

        T value = std::move(state->queue.front());
        state->queue.pop_front();
        return option::Option<T>::some(std::move(value));
    }

    [[nodiscard]] option::Option<T> tryReceive() const
    {
        ensureLive();

        std::scoped_lock lock(state->mutex);
        if (state->queue.empty())
        {
            return option::Option<T>::none();
        }

        T value = std::move(state->queue.front());
        state->queue.pop_front();
        return option::Option<T>::some(std::move(value));
    }

    [[nodiscard]] bool isClosed() const
    {
        ensureLive();

        std::scoped_lock lock(state->mutex);
        return state->closed;
    }

private:
    template <typename U>
    friend class Channel;

    explicit Receiver(std::shared_ptr<internal::ChannelState<T>> owned)
        : state(std::move(owned))
    {
    }

    void ensureLive() const
    {
        if (!state)
        {
            internal::failMovedFrom();
        }
    }

    std::shared_ptr<internal::ChannelState<T>> state;
};

template <typename T>
class Channel final
{
public:
    [[nodiscard]] static std::pair<Sender<T>, Receiver<T>> create()
    {
        auto shared = std::make_shared<internal::ChannelState<T>>();
        return {Sender<T>(shared), Receiver<T>(shared)};
    }
};
} // namespace safe::concurrency
