# Borrowing

`BorrowCell<T>` provides runtime enforcement of the core shared-versus-exclusive borrowing rule. Any number of `Ref<T>` shared borrows may coexist, or one `Mut<T>` exclusive borrow may exist, but not both. Conflicting requests raise a borrow violation. `tryBorrow` and `tryBorrowMut` express contention through `Option`.

Borrow handles own shared internal state, so moving or destroying the original `BorrowCell` does not leave an active handle dangling. This is runtime enforcement, not a compile-time borrow checker.
