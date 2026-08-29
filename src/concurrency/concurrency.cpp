#include <safe_cpp/concurrency/channel.hpp>
#include <safe_cpp/diagnostics/diagnostics.hpp>

#include <exception>

namespace safe::concurrency::internal
{
[[noreturn]] void failMovedFrom()
{
    diagnostics::fail(diagnostics::ViolationKind::movedFrom, "Attempted to use a moved-from channel endpoint");
}
} // namespace safe::concurrency::internal

namespace safe::concurrency
{
std::string_view channelErrorMessage(ChannelError error) noexcept
{
    switch (error)
    {
    case ChannelError::closed:
        return "Channel is closed";
    }

    std::terminate();
}
} // namespace safe::concurrency
