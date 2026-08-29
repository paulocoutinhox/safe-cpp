#include <safe_cpp/arithmetic/checked_integer.hpp>
#include <safe_cpp/diagnostics/diagnostics.hpp>

namespace safe::arithmetic::internal
{
[[noreturn]] void failOverflow()
{
    diagnostics::fail(diagnostics::ViolationKind::arithmetic, "Checked arithmetic overflowed");
}

[[noreturn]] void failDivisionByZero()
{
    diagnostics::fail(diagnostics::ViolationKind::arithmetic, "Checked arithmetic attempted division by zero");
}
} // namespace safe::arithmetic::internal
