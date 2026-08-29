# C++ version behavior

C++20 is the minimum supported language level. The public API does not fork by standard version. Newer standard-library facilities may replace internal implementations when they provide the same semantics cleanly.

C++23 uses `std::expected` when `__cpp_lib_expected` reports support. C++20 uses the internal `Result` storage. Consumers keep the same `safe::result::Result<T, E>` API in both modes.
