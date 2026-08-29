#include <safe_cpp/defer/defer.hpp>

#include <cstdlib>

namespace safe::scope::internal
{
[[noreturn]] void terminateFromDeferredAction() noexcept
{
    std::terminate();
}
} // namespace safe::scope::internal
