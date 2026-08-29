#include <safe_cpp/diagnostics/diagnostics.hpp>
#include <safe_cpp/option/option.hpp>

namespace safe::option::internal
{
[[noreturn]] void failEmptyOption()
{
    diagnostics::fail(diagnostics::ViolationKind::option, "Attempted to access an empty Option");
}
} // namespace safe::option::internal
