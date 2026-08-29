#include <safe_cpp/bounds/bounds.hpp>
#include <safe_cpp/diagnostics/diagnostics.hpp>

#include <string>

namespace safe::bounds::internal
{
[[noreturn]] void failOutOfRange(std::size_t index, std::size_t size)
{
    diagnostics::fail(
        diagnostics::ViolationKind::bounds,
        "Index " + std::to_string(index) + " is outside a sequence of size " + std::to_string(size));
}
} // namespace safe::bounds::internal

namespace safe::bounds
{
char &String::at(std::size_t index)
{
    if (index >= stored.size())
    {
        internal::failOutOfRange(index, stored.size());
    }

    return stored[index];
}

const char &String::at(std::size_t index) const
{
    if (index >= stored.size())
    {
        internal::failOutOfRange(index, stored.size());
    }

    return stored[index];
}
} // namespace safe::bounds
