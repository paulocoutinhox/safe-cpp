# Arithmetic

`CheckedInteger<T>` wraps non-bool integral values and validates addition, subtraction, multiplication, division, remainder and negation. Overflow and division by zero become deterministic arithmetic violations. The implementation uses compiler overflow intrinsics where available and a portable checked implementation otherwise. `bool` is not treated as an arithmetic type.
