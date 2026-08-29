#include <safe_cpp/diagnostics/diagnostics.hpp>
#include <safe_cpp/result/result.hpp>

namespace safe::result::internal
{
[[noreturn]] void failMissingValue()
{
    diagnostics::fail(diagnostics::ViolationKind::result, "Attempted to access the value of an error Result");
}

[[noreturn]] void failMissingError()
{
    diagnostics::fail(diagnostics::ViolationKind::result, "Attempted to access the error of a successful Result");
}
} // namespace safe::result::internal
