# Getting started

The project requires C++20 or newer and CMake 3.20 or newer. Link `safe-cpp::safe-cpp`, include either `<safe_cpp/safe_cpp.hpp>` or only the module headers you need, and keep tests disabled when consuming the library as a dependency.

For local development, run `python3 make.py all`. The command configures the project, builds the library and tests, then executes CTest.

The library treats `Result` as expected domain failure, `Option` as explicit absence and `diagnostics::Violation` as a detected programming safety violation.
