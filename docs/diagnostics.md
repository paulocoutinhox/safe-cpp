# Diagnostics

Detected programming safety violations use `safe::diagnostics::Violation`. Every violation carries a `ViolationKind` so callers and tests can distinguish allocation, arithmetic, borrowing, bounds, moved-from access, nullability, option and result failures without parsing message text.

Diagnostics are for violated safe-cpp contracts. Domain failures belong in `Result`.
