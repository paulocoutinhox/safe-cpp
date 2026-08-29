#pragma once

#include <string_view>

namespace safe::version
{
[[nodiscard]] std::string_view string() noexcept;
[[nodiscard]] int languageLevel() noexcept;
} // namespace safe::version
