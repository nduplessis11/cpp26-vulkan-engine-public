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

    const auto [steps, alpha] = frame_clock.advance(kSandboxConfig.fixed_step);

    EXPECT_EQ(steps, 1U);
}

// TODO(human): the second failing case — time left over survives the call.
// Name it TEST(FrameClockAdvance, CarriesTheRemainderIntoTheNextCall).
