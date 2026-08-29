# Allocation

`safe::allocation` makes allocator ownership explicit. `HeapAllocator` performs aligned heap allocation. `ArenaAllocator` uses a monotonic PMR resource for grouped allocations. Both create typed `Allocation<T>` objects through `make<T>()`.

An `Allocation<T>` keeps its allocator alive, constructs the object once and destroys it automatically. Raw byte allocation remains private to allocator implementations.

`Allocator::make` requires the allocator to be owned by `std::shared_ptr`, which is what `create()` provides. `ArenaAllocator` does not reclaim individual allocations. Destroying an `Allocation<T>` runs the destructor, and the arena memory is released only when the allocator itself is destroyed.
