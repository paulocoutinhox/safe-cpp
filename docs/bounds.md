# Bounds

`safe::bounds` contains `Vector`, `Array`, `Slice` and `String`. Their indexed access checks bounds before returning an element and raises a bounds violation for invalid indices.

`Slice<T>` is intentionally a non-owning standard C++ view. A const `Slice<T>` yields const element access. It guarantees index validation but standard C++ cannot prove that the source outlives the slice. Lifetime proof requires compiler support.
