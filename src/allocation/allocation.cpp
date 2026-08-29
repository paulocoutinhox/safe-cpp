#include <safe_cpp/allocation/allocation.hpp>
#include <safe_cpp/diagnostics/diagnostics.hpp>

namespace safe::allocation::internal
{
[[noreturn]] void failMovedFrom()
{
    diagnostics::fail(diagnostics::ViolationKind::movedFrom, "Attempted to access a moved-from allocation");
}

[[noreturn]] void failUnsharedAllocator()
{
    diagnostics::fail(diagnostics::ViolationKind::allocation, "Allocator::make requires a shared allocator");
}

[[noreturn]] void failNullAllocation()
{
    diagnostics::fail(diagnostics::ViolationKind::nullPointer, "Allocator::make received a null allocation");
}
} // namespace safe::allocation::internal

namespace safe::allocation
{
std::shared_ptr<HeapAllocator> HeapAllocator::create()
{
    return std::shared_ptr<HeapAllocator>(new HeapAllocator());
}

void *HeapAllocator::allocateBytes(std::size_t size, std::size_t alignment)
{
    return ::operator new(size, std::align_val_t(alignment));
}

void HeapAllocator::deallocateBytes(void *pointer, std::size_t, std::size_t alignment) noexcept
{
    ::operator delete(pointer, std::align_val_t(alignment));
}

std::shared_ptr<ArenaAllocator> ArenaAllocator::create(std::size_t initialSize)
{
    return std::shared_ptr<ArenaAllocator>(new ArenaAllocator(initialSize));
}

ArenaAllocator::ArenaAllocator(std::size_t initialSize)
    : initialBuffer(std::make_unique<std::byte[]>(initialSize))
    , resource(initialBuffer.get(), initialSize)
{
}

void *ArenaAllocator::allocateBytes(std::size_t size, std::size_t alignment)
{
    std::scoped_lock lock(mutex);
    return resource.allocate(size, alignment);
}

void ArenaAllocator::deallocateBytes(void *pointer, std::size_t size, std::size_t alignment) noexcept
{
    std::scoped_lock lock(mutex);
    resource.deallocate(pointer, size, alignment);
}
} // namespace safe::allocation
