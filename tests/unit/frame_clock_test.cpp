// Every case hands `advance` a duration directly, so no case reads a clock or sleeps.
// Step counts and remainders are integers and compare exactly; alpha is a double
// and needs a tolerance.
#include <chrono>
#include <engine/core/frame_clock.hpp>
#include <gtest/gtest.h>

using engine::core::FrameClock;
using engine::core::FrameClockConfig;

using namespace std::chrono_literals;

namespace {

// The configuration the sandbox runs with. The cap of 5 binds long before the
// 250ms clamp does; a case that wants to watch the clamp builds its own config.
constexpr FrameClockConfig kSandboxConfig{
    .fixed_step = 10ms,
    .max_elapsed = 250ms,
    .max_steps = 5,
};

} // namespace

TEST(FrameClockAdvance, OneWholeStepRunsOneUpdate) {
    FrameClock frame_clock(kSandboxConfig);

    auto const [steps, alpha] = frame_clock.advance(10ms);

    EXPECT_EQ(steps, 1U);
}

TEST(FrameClockAdvance, CarriesTheRemainderIntoTheNextCall) {
    FrameClock frame_clock(kSandboxConfig);

    auto const [first_steps, first_alpha] = frame_clock.advance(7ms);
    auto const [second_steps, second_alpha] = frame_clock.advance(7ms);

    EXPECT_EQ(first_steps, 0U);
    EXPECT_EQ(second_steps, 1U);
}

TEST(FrameClockAdvance, AlphaIsTheFractionOfAStepLeftOver) {
    FrameClock frame_clock(kSandboxConfig);

    auto const [steps, alpha] = frame_clock.advance(13ms);

    ASSERT_EQ(steps, 1U);
    EXPECT_DOUBLE_EQ(alpha, 0.3);
}

// TODO(human): one elapsed duration long enough to run several whole steps.
// Name it TEST(FrameClockAdvance, RunsOneStepPerWholeFixedStepOfElapsedTime).
