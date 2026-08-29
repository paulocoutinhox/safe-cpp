#include <safe_cpp/allocation/allocation.hpp>
#include <safe_cpp/arithmetic/checked_integer.hpp>
#include <safe_cpp/borrow/borrow.hpp>
#include <safe_cpp/bounds/bounds.hpp>
#include <safe_cpp/diagnostics/diagnostics.hpp>
#include <safe_cpp/memory/memory.hpp>
#include <safe_cpp/option/option.hpp>
#include <safe_cpp/result/result.hpp>

#include <gtest/gtest.h>

#include <limits>
#include <string>
#include <utility>

namespace safe::diagnostics::tests
{
TEST(DiagnosticsTest, FailThrowsRequestedKind)
{
    try
    {
        fail(ViolationKind::allocation, "Allocator::make requires a shared allocator");
        FAIL();
    }
    catch (const Violation &violation)
    {
        EXPECT_EQ(violation.kind(), ViolationKind::allocation);
        EXPECT_STREQ(violation.what(), "Allocator::make requires a shared allocator");
    }
}

TEST(DiagnosticsTest, ReportsEveryViolationKind)
{
    try
    {
        option::Option<int>::none().value();
        FAIL();
    }
    catch (const Violation &violation)
    {
        EXPECT_EQ(violation.kind(), ViolationKind::option);
    }

    try
    {
        static_cast<void>(result::Result<int, std::string>::err("failed").value());
        FAIL();
    }
    catch (const Violation &violation)
    {
        EXPECT_EQ(violation.kind(), ViolationKind::result);
    }

    try
    {
        static_cast<void>(memory::NonNull<int>(static_cast<int *>(nullptr)));
        FAIL();
    }
    catch (const Violation &violation)
    {
        EXPECT_EQ(violation.kind(), ViolationKind::nullPointer);
    }

    try
    {
        const arithmetic::CheckedInteger<int> maximum(std::numeric_limits<int>::max());
        static_cast<void>(maximum + arithmetic::CheckedInteger<int>(1));
        FAIL();
    }
    catch (const Violation &violation)
    {
        EXPECT_EQ(violation.kind(), ViolationKind::arithmetic);
    }

    try
    {
        bounds::Vector<int> values{1};
        static_cast<void>(values[1]);
        FAIL();
    }
    catch (const Violation &violation)
    {
        EXPECT_EQ(violation.kind(), ViolationKind::bounds);
    }

    try
    {
        borrow::BorrowCell<int> value(42);
        const auto shared = value.borrow();
        static_cast<void>(value.borrowMut());
        FAIL();
    }
    catch (const Violation &violation)
    {
        EXPECT_EQ(violation.kind(), ViolationKind::borrow);
    }

    try
    {
        fail(ViolationKind::allocation, "Allocator::make requires a shared allocator");
        FAIL();
    }
    catch (const Violation &violation)
    {
        EXPECT_EQ(violation.kind(), ViolationKind::allocation);
    }

    try
    {
        auto allocator = allocation::HeapAllocator::create();
        auto first = allocator->make<int>(1);
        auto second = std::move(first);
        EXPECT_EQ(*second, 1);
        static_cast<void>(*first);
        FAIL();
    }
    catch (const Violation &violation)
    {
        EXPECT_EQ(violation.kind(), ViolationKind::movedFrom);
    }
}
} // namespace safe::diagnostics::tests
