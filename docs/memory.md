# Memory

`safe::memory` provides ownership-oriented primitives. `Box<T>` owns exactly one object. `Arc<T>` provides shared immutable access. `Weak<T>` observes an `Arc<T>` without extending its lifetime and upgrades through `Option<Arc<T>>`. `NonNull<T>` represents a pointer that is validated as non-null at construction. It does not extend or track the lifetime of the pointee.

These types improve ownership expression but cannot stop arbitrary raw-pointer code written elsewhere in a C++ program.
