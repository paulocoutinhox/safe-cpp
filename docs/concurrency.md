# Concurrency

`Mutex<T>` combines a mutex and its protected value. `withLock` executes a callback while holding the lock and rejects direct reference and pointer return types. That blocks the obvious return path. A callback can still copy an address into captured state or return a wrapper that carries one, such as `std::reference_wrapper`, `std::span` or an iterator, so the lock does not prove that protected data cannot escape.

`Channel<T>::create()` returns a `Sender<T>` and a `Receiver<T>`. Send failures use `Result<void, ChannelError>`. `tryReceive` returns empty when the queue is empty, including on an open channel, and still drains remaining values after close. `isClosed` distinguishes a closed channel from a temporary empty queue. A closed and drained `receive` returns an empty `Option<T>`. Closure is explicit. Using a moved-from sender or receiver is a deterministic violation.
