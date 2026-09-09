// Tests for FakeDriver and FakeHandle.
//
// Each test creates its own driver so the counters start at zero.
// FakeHandle tests use those counters to verify that handles are closed.
#include <engine/core/fake_handle.hpp>
#include <gtest/gtest.h>
#include <type_traits>
#include <utility>

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

// --- FakeHandle: move construction ------------------------------------------

TEST(FakeHandle, MoveConstructionTransfersOwnership) {
    FakeDriver driver;
    {
        FakeHandle source{driver};
        FakeHandle moved{std::move(source)};

        // Reading the moved-from handle is the assertion: the move must empty it.
        // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
        EXPECT_FALSE(source.is_valid());
        EXPECT_TRUE(moved.is_valid());
    }
    EXPECT_EQ(driver.close_count, 1u);
}

// --- FakeHandle: move assignment --------------------------------------------

TEST(FakeHandle, MoveAssignmentClosesTheTargetsOldId) {
    FakeDriver driver;
    {
        FakeHandle source{driver};
        FakeHandle moved{driver};
        moved = std::move(source);

        // Reading the moved-from handle is the assertion: the move must empty it.
        // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
        EXPECT_FALSE(source.is_valid());
        EXPECT_TRUE(moved.is_valid());
    }
    EXPECT_EQ(driver.close_count, 2u);
}

TEST(FakeHandle, SelfMoveAssignmentKeepsTheHandleValid) {
    FakeDriver driver;
    {
        FakeHandle source{driver};
        FakeHandle& alias = source; // no compiler warning

        source = std::move(alias);

        EXPECT_TRUE(source.is_valid());
    }
    EXPECT_EQ(driver.close_count, 1u);
}

// --- FakeHandle: reset ------------------------------------------------------

TEST(FakeHandle, ResetClosesValidHandle) {
    FakeDriver driver;
    {
        FakeHandle handle{driver};
        ASSERT_TRUE(handle.is_valid());

        handle.reset();

        EXPECT_EQ(driver.close_count, 1u);
    }
}

TEST(FakeHandle, DestructorAfterResetDoesNotCloseAgain) {
    FakeDriver driver;
    {
        FakeHandle handle{driver};
        ASSERT_TRUE(handle.is_valid());
        handle.reset();
        ASSERT_EQ(driver.close_count, 1u);
    }
    EXPECT_EQ(driver.close_count, 1u);
}

TEST(FakeHandle, ResetTwiceOnlyClosesHandleOnce) {
    FakeDriver driver;
    {
        FakeHandle handle{driver};
        ASSERT_TRUE(handle.is_valid());

        handle.reset();
        handle.reset();

        EXPECT_EQ(driver.close_count, 1u);
    }
}

// --- FakeHandle: empty state ------------------------------------------------

TEST(FakeHandle, EmptyHandleClosesNothing) {
    FakeDriver driver;
    {
        FakeHandle handle;
        ASSERT_FALSE(handle.is_valid());
    }
    EXPECT_EQ(driver.close_count, 0u);
}

// --- FakeHandle: get --------------------------------------------------------

TEST(FakeHandle, GetReturnsTheOwnedId) {
    FakeDriver driver;
    FakeHandle handle{driver};

    EXPECT_EQ(handle.get(), 1u);
}

TEST(FakeHandleDeathTest, RejectsGetOnEmptyHandle) {
    FakeHandle handle;
    ASSERT_FALSE(handle.is_valid());

    EXPECT_DEATH(static_cast<void>(handle.get()), "is_valid");
}
