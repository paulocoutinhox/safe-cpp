#include <safe_cpp/diagnostics/diagnostics.hpp>
#include <safe_cpp/memory/memory.hpp>

namespace safe::memory::internal
{
[[noreturn]] void failNullPointer()
{
    diagnostics::fail(diagnostics::ViolationKind::nullPointer, "A non-null memory type received a null pointer");
}

[[noreturn]] void failMovedFrom()
{
    diagnostics::fail(diagnostics::ViolationKind::movedFrom, "Attempted to access a moved-from memory type");
}
} // namespace safe::memory::internal
