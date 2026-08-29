# Result

`safe::result::Result<T, E>` represents either a value or an error without exceptions for expected domain failures. C++23 standard libraries with `std::expected` use it internally. C++20 uses the safe-cpp backend while preserving the same public API.

Use `Result<T, E>::ok(value)` and `Result<T, E>::err(error)`. `Result<void, E>` represents operations that only need success or failure. `SAFE_CPP_TRY(name, expression)` unwraps a successful `Result` with a value. `SAFE_CPP_TRY_VOID(expression)` propagates failure from `Result<void, E>`.
