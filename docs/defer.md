# Defer

`defer { ... };` schedules a block to run exactly once when the current scope exits. `makeDefer(function)` provides the same mechanism without macro syntax. A finalizer can be cancelled explicitly.

The convenience syntax requires a macro because standard C++ has no `defer` keyword. Deferred actions execute from a `noexcept` destructor. If a deferred action throws, the process terminates rather than allowing an exception to escape a destructor.
