# safe-cpp

## Purpose

This library is a C++20+ project that provides safety-oriented APIs that follow approaches from languages with newer safety mechanics and features while remaining standard C++ and preserving normal C++ ABI and tooling. The project must never claim that a library can reproduce compile-time lifetime guarantees that require compiler support.

## Architecture

The public API lives under `include/safe_cpp`. Every feature has a dedicated package and namespace. Runtime implementation files mirror those packages under `src`. No anonymous namespaces are used.

The supported packages are:

- `safe::allocation` for explicit typed allocation and arena ownership.
- `safe::arithmetic` for checked integer operations.
- `safe::borrow` for runtime shared and exclusive borrowing.
- `safe::bounds` for checked sequence access.
- `safe::concurrency` for protected shared state and channels.
- `safe::scope` for scope finalization.
- `safe::diagnostics` for deterministic library safety violations.
- `safe::memory` for ownership and non-null memory types.
- `safe::option` for optional state.
- `safe::result` for structured success and failure.
- `safe::version` for library and active language information.

`include/safe_cpp/safe_cpp.hpp` is the convenience umbrella header. Individual package headers remain independently usable.

## Language levels

C++20 is the minimum language level. The project must compile cleanly as C++20 and newer.

C++23 behavior is selected only when the compiler and standard library actually expose the corresponding standard feature. `Result<T, E>` uses `std::expected` when `__cpp_lib_expected` is available at the required level and otherwise uses the C++20 backend with the same safe-cpp API.

Do not add compatibility branches for obsolete library APIs. Version-specific code exists only when a newer C++ standard provides a materially better implementation of the same current API.

## Safety rules

Safe APIs should make invalid states explicit and deterministic. Do not silently continue after a failed safety precondition. Do not invent fallback values for invalid state.

Prefer ownership-bearing types over raw ownership. Raw pointers may exist internally when they are required to implement a primitive, but ownership must remain represented by a class with deterministic lifetime.

Do not expose nullable values through types that claim non-null semantics. Do not return references or pointers from synchronization callbacks when doing so could let protected state escape its synchronization lifetime.

Bounds-checked types must validate every indexed access exposed through their primary API. Checked arithmetic must detect overflow before relying on undefined signed overflow behavior.

Borrowing in standard C++ cannot provide compile-time lifetime proofs. `BorrowCell<T>` therefore enforces the core shared-versus-exclusive rule at runtime and keeps the borrowed object alive through shared state. Documentation and names must state this accurately.

## Error model

Programming contract violations detected by safe-cpp use `safe::diagnostics::Violation` with a `ViolationKind`. These checks replace silent unsafe behavior in safe-cpp APIs with deterministic failure.

Expected domain failures use `Result<T, E>`. Optional absence uses `Option<T>`. Do not use a sentinel value when the absence or error state deserves a type.

## Defer

The `defer { ... };` syntax is intentionally implemented with a macro because standard C++ has no keyword or equivalent syntax. `makeDefer()` is the macro-free API. A deferred action runs once at scope exit unless cancelled. Exceptions escaping a deferred action terminate the process because destructors are `noexcept`.

Lambdas must be placed between `// clang-format off` and `// clang-format on` and manually formatted consistently.

## Allocation

`Allocator` instances are shared owners so an `Allocation<T>` keeps its allocator alive. `HeapAllocator` uses aligned global allocation. `ArenaAllocator` uses `std::pmr::monotonic_buffer_resource` and serializes allocation access.

Typed allocations construct and destroy objects through RAII. Public callers should not need to manually call raw allocation or deallocation functions.

## Concurrency

`Mutex<T>` owns both the mutex and protected value. Access happens through `withLock`. Direct reference and pointer return types are rejected. A callback can still store an address in captured state or return a wrapper that carries one.

Channels use explicit closure. Sending to a closed channel returns `Result<void, ChannelError>`. Receiving from a closed and drained channel returns an empty `Option<T>`.

## Project layout

- `include/` contains public headers only.
- `src/` contains runtime implementation grouped by feature.
- `tests/` contains GoogleTest and GoogleMock tests.
- `examples/` contains buildable usage examples.
- `docs/` contains one focused document per module or project concern.
- `cmake/` contains package configuration templates only.
- `extras/images/` contains project logo assets.
- `.github/workflows/` contains CI.
- `make.py` is the dependency-free Python task runner.

Do not add generated files, migration files, compatibility scripts or alternative build systems.

## CMake

The canonical library target is `safe-cpp::safe-cpp`. Consumers must be able to use the project through `add_subdirectory`, `FetchContent` or an installed CMake package.

Tests are disabled by default for consumers and enabled with `SAFE_CPP_BUILD_TESTS`. Examples are disabled by default and enabled with `SAFE_CPP_BUILD_EXAMPLES`. AddressSanitizer and UndefinedBehaviorSanitizer are opt-in through `SAFE_CPP_ENABLE_SANITIZERS`. ThreadSanitizer is opt-in through `SAFE_CPP_ENABLE_THREAD_SANITIZER` and cannot be combined with AddressSanitizer. Coverage instrumentation is opt-in through `SAFE_CPP_ENABLE_COVERAGE`.

Tests use GoogleTest and GoogleMock and are registered with CTest through `gtest_discover_tests`.

## make.py

`make.py` uses only the Python standard library. Its default command is `help`. Tasks shell out to CMake, CTest and clang-format. It must not become a second build system. CMake remains authoritative.

Expected tasks are `help`, `configure`, `build`, `test`, `format`, `format-check`, `clean` and `all`.

## Formatting

`.clang-format` is authoritative. Code should remain compact, deliberate and easy to scan.

Separate different responsibilities with one blank line. Avoid vertical padding that does not improve readability. Prefer early returns when they simplify control flow. Do not add an `else` after an unconditional return.

Comments are rare. Every normal comment is a complete English sentence beginning with a capital letter and ending with a period. Comments explain intent or context rather than narrating code. Do not add comments in headers merely to describe members, methods or sections.

## Naming

Code, API names, comments, commit-oriented text and documentation examples use English. Namespaces are explicit and feature-scoped. Anonymous namespaces are forbidden.

Classes use `PascalCase`. Functions, methods and variables use `camelCase`. Variable names must not start or end with an underscore. Macros use `SAFE_CPP_` prefixes except the intentional `defer` convenience macro.

## Testing

Every behavior change must keep the existing test suite passing across C++20 and C++23. Tests should cover success paths and deterministic safety violations. Do not add tests that depend on undefined behavior to prove safety.

CI uses GCC and Clang on Ubuntu, Apple Clang on macOS and MSVC on Windows. CTest is the test runner exposed by the build.

## Limitations

Standard C++20 and C++23 do not expose a language-level borrow checker, lifetime proof system, mandatory safe and unsafe contexts, or compiler-enforced thread-transfer and shared-access rules. Do not fake those guarantees with names or documentation.

`Slice<T>` is a non-owning view because that is what standard C++ can express without changing object ownership. It guarantees checked indexing but cannot prove the lifetime of its source at compile time.

References returned from ordinary C++ containers can still become invalid after mutation because the language has no borrow checker. Library wrappers reduce common hazards but cannot prevent arbitrary unsafe standard C++ written outside their APIs.
