#include <safe_cpp/diagnostics/diagnostics.hpp>

namespace safe::diagnostics
{
Violation::Violation(ViolationKind kind, std::string message)
    : std::logic_error(std::move(message))
    , storedKind(kind)
{
}

ViolationKind Violation::kind() const noexcept
{
    return storedKind;
}

[[noreturn]] void fail(ViolationKind kind, std::string_view message)
{
    throw Violation(kind, std::string(message));
}
} // namespace safe::diagnostics
