#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace safe::diagnostics
{
enum class ViolationKind
{
    allocation,
    arithmetic,
    borrow,
    bounds,
    movedFrom,
    nullPointer,
    option,
    result
};

class Violation final : public std::logic_error
{
public:
    Violation(ViolationKind kind, std::string message);

    [[nodiscard]] ViolationKind kind() const noexcept;

private:
    ViolationKind storedKind;
};

[[noreturn]] void fail(ViolationKind kind, std::string_view message);
} // namespace safe::diagnostics
