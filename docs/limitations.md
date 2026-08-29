# Limitations

The library cannot make arbitrary standard C++ as memory-safe as a language with compiler-enforced safety because a library cannot add compile-time lifetime proofs, language-enforced ownership, a borrow checker, mandatory `unsafe` boundaries or compiler-enforced thread-transfer and shared-access rules.

The project therefore distinguishes hard library guarantees from compiler-level guarantees it does not have. Runtime borrow enforcement, ownership classes, checked bounds, checked arithmetic and concurrency wrappers reduce important classes of failures while remaining honest about the remaining language surface.

`Mutex<T>::withLock` cannot stop a callback from storing an address of protected data in captured state or from returning a wrapper that carries one. `Slice<T>` is a non-owning view and cannot prove that its source outlives the slice. `NonNull<T>` rejects a null address at construction and does not track whether the pointee remains alive.

Code that deliberately bypasses safe-cpp with raw pointers, unchecked standard containers, invalid references, unsafe casts or data races remains subject to normal C++ rules.
