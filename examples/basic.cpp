#include <safe_cpp/safe_cpp.hpp>

#include <cstdint>
#include <iostream>
#include <string>

namespace safe_cpp_example
{
using safe::allocation::HeapAllocator;
using safe::result::Result;

Result<std::string, std::string> makeGreeting(std::string name)
{
    if (name.empty())
    {
        return Result<std::string, std::string>::err("Name cannot be empty");
    }

    return Result<std::string, std::string>::ok("Hello, " + std::move(name));
}

Result<void, std::string> run()
{
    auto allocator = HeapAllocator::create();

    SAFE_CPP_TRY(greeting, makeGreeting("world"));

    auto number = allocator->make<std::uint8_t>(std::uint8_t{255});

    // clang-format off
    defer
    {
        std::cout << "Deferred cleanup completed\n";
    };
    // clang-format on

    std::cout << greeting << '\n';
    std::cout << "Allocated value: " << static_cast<int>(*number) << '\n';

    return Result<void, std::string>::ok();
}
} // namespace safe_cpp_example

int main()
{
    const auto result = safe_cpp_example::run();
    return result ? 0 : 1;
}
