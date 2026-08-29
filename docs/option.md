# Option

`safe::option::Option<T>` makes optional state explicit. Use `Option<T>::some(value)` for presence and `Option<T>::none()` for absence. Accessing `value()` on an empty option raises a typed safety violation instead of returning a sentinel or continuing with invalid state.

`map` transforms only present values and preserves absence.
