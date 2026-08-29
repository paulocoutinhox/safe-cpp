#include <safe_cpp/config/config.hpp>
#include <safe_cpp/version/version.hpp>

namespace safe::version
{
std::string_view string() noexcept
{
    return "0.1.0";
}

int languageLevel() noexcept
{
    return SAFE_CPP_LANGUAGE_LEVEL;
}
} // namespace safe::version
