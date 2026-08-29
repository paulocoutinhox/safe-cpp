#include <safe_cpp/borrow/borrow.hpp>
#include <safe_cpp/diagnostics/diagnostics.hpp>

namespace safe::borrow::internal
{
[[noreturn]] void failSharedBorrow()
{
    diagnostics::fail(diagnostics::ViolationKind::borrow, "A shared borrow was requested while a mutable borrow is active");
}

[[noreturn]] void failMutableBorrow()
{
    diagnostics::fail(diagnostics::ViolationKind::borrow, "A mutable borrow was requested while another borrow is active");
}

[[noreturn]] void failMovedFrom()
{
    diagnostics::fail(diagnostics::ViolationKind::movedFrom, "Attempted to access a moved-from borrow");
}
} // namespace safe::borrow::internal
