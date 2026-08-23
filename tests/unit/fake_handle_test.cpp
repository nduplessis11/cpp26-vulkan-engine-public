// Tests for FakeDriver and FakeHandle.
//
// Each test creates its own driver so the counters start at zero.
// FakeHandle tests use those counters to verify that handles are closed.
#include <engine/core/fake_handle.hpp>
#include <gtest/gtest.h>
#include <type_traits>

using engine::core::fake_close;
using engine::core::fake_open;
using engine::core::FakeDriver;
using engine::core::FakeHandle;

// A copy makes two owners of one id, and both destructors close it.
static_assert(!std::is_copy_constructible_v<FakeHandle>);
static_assert(!std::is_copy_assignable_v<FakeHandle>);

// --- FakeDriver: id issuance ------------------------------------------------

// ID 0 is reserved for an empty FakeHandle, so real handles start at 1.
TEST(FakeDriver, OpenIssuesDistinctIdsStartingAtOne) {
    FakeDriver driver;

    EXPECT_EQ(fake_open(driver), 1u);
    EXPECT_EQ(fake_open(driver), 2u);
    EXPECT_EQ(driver.open_count, 2u);
}

// --- FakeDriver: close accounting -------------------------------------------

TEST(FakeDriver, CloseIncrementsCloseCount) {
    FakeDriver driver;
    auto handle = fake_open(driver);

    auto before_close = driver.close_count;
    fake_close(driver, handle);

    EXPECT_EQ(driver.close_count, before_close + 1);
}

// IDs are retired on close, never recycled. A stale ID must remain invalid
// so use-after-close cannot accidentally target a new live handle.
TEST(FakeDriver, OpenDoesNotReuseIds) {
    FakeDriver driver;

    auto first = fake_open(driver);
    fake_close(driver, first);
    auto second = fake_open(driver);

    EXPECT_NE(first, second);
}

// --- FakeHandle: destruction ------------------------------------------------

// Code that holds a FakeHandle never calls fake_close itself, so it cannot
// skip the close on an early return.
TEST(FakeHandle, DestructionClosesIdOnce) {
    FakeDriver driver;
    {
        FakeHandle handle{driver};
    }

    EXPECT_EQ(driver.close_count, 1u);
}
