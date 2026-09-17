// Every case hands `advance` a duration directly, so no case reads a clock or sleeps.
// Step counts and remainders are integers and compare exactly; alpha is a double
// and needs a tolerance.
#include <array>
#include <chrono>
#include <cstddef>
#include <engine/core/frame_clock.hpp>
#include <gtest/gtest.h>
#include <vector>

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

struct SimCounter {
    std::size_t updates{0};
    auto step() -> void {
        ++updates;
    }
};

struct FakeRenderTarget {
    std::vector<double> alphas;
    auto render(double alpha) -> void {
        alphas.push_back(alpha);
    }
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

TEST(FrameClockAdvance, RunsOneStepPerWholeFixedStepOfElapsedTime) {
    FrameClock frame_clock(kSandboxConfig);

    auto const [steps, alpha] = frame_clock.advance(50ms);

    EXPECT_EQ(steps, 5U);
    EXPECT_DOUBLE_EQ(alpha, 0.0);
}

TEST(FrameClockAdvance, StopsAtMaxStepsAndDiscardsTheTimeItSkipped) {
    FrameClock frame_clock(kSandboxConfig);

    auto const [steps, alpha] = frame_clock.advance(246ms);

    EXPECT_EQ(steps, 5U);
    EXPECT_DOUBLE_EQ(alpha, 0.6);
}

TEST(FrameClockAdvance, ClampsElapsedToMaxElapsedBeforeAccumulating) {
    FrameClock frame_clock(kSandboxConfig);

    auto const [steps, alpha] = frame_clock.advance(309ms);

    EXPECT_EQ(steps, 5U);
    EXPECT_DOUBLE_EQ(alpha, 0.0);
}

TEST(FrameClockAdvance, TreatsNegativeElapsedAsNoTimePassing) {
    FrameClock frame_clock(kSandboxConfig);

    auto const [steps, alpha] = frame_clock.advance(-5ms);

    EXPECT_EQ(steps, 0U);
    EXPECT_DOUBLE_EQ(alpha, 0.0);
}

TEST(FrameClockConstructorDeathTest, RejectsZeroFixedStep) {
    constexpr FrameClockConfig kSandboxZeroStepConfig{
        .fixed_step = 0ms,
        .max_elapsed = 250ms,
        .max_steps = 5,
    };

    EXPECT_DEATH(FrameClock{kSandboxZeroStepConfig}, "fixed_step");
}

TEST(FrameClockConstructorDeathTest, RejectsZeroMaxSteps) {
    constexpr FrameClockConfig kSandboxZeroMaxStepConfig{
        .fixed_step = 10ms,
        .max_elapsed = 250ms,
        .max_steps = 0,
    };

    EXPECT_DEATH(FrameClock{kSandboxZeroMaxStepConfig}, "max_steps");
}

// The lecture's four-frame table, run on one clock. A clock that drops the remainder
// between calls returns 0, 0, 3, 0 steps instead of 0, 1, 3, 1.
TEST(FrameClockAdvance, RunsTheWorkedSequence) {
    struct Frame {
        std::chrono::nanoseconds elapsed;
        std::size_t steps;
        std::chrono::nanoseconds remainder;
        double alpha;
    };

    constexpr std::array<Frame, 4> kFrames{{
        {.elapsed = 6ms, .steps = 0U, .remainder = 6ms, .alpha = 0.6},
        {.elapsed = 7ms, .steps = 1U, .remainder = 3ms, .alpha = 0.3},
        {.elapsed = 32ms, .steps = 3U, .remainder = 5ms, .alpha = 0.5},
        {.elapsed = 5ms, .steps = 1U, .remainder = 0ms, .alpha = 0.0},
    }};

    FrameClock frame_clock(kSandboxConfig);

    for (std::size_t i = 0; i < kFrames.size(); ++i) {
        SCOPED_TRACE(testing::Message() << "frame " << i + 1);
        auto const [steps, alpha] = frame_clock.advance(kFrames[i].elapsed);

        EXPECT_EQ(steps, kFrames[i].steps);
        EXPECT_EQ(frame_clock.remainder(), kFrames[i].remainder);
        EXPECT_DOUBLE_EQ(alpha, kFrames[i].alpha);
    }
}

TEST(FrameClockAdvance, DrivesUpdatesAndRendersAlphaOverTheWorkedSequence) {
    struct Frame {
        std::chrono::nanoseconds elapsed;
        double alpha;
    };

    constexpr std::array<Frame, 4> kFrames{{
        {.elapsed = 6ms, .alpha = 0.6},
        {.elapsed = 7ms, .alpha = 0.3},
        {.elapsed = 32ms, .alpha = 0.5},
        {.elapsed = 5ms, .alpha = 0.0},
    }};

    FrameClock frame_clock{kSandboxConfig};

    SimCounter sim;
    FakeRenderTarget target;
    for (std::size_t i = 0; i < kFrames.size(); ++i) {
        SCOPED_TRACE(testing::Message() << "frame " << i + 1);

        auto const [steps, alpha] = frame_clock.advance(kFrames[i].elapsed);
        for (std::size_t j = 0; j < steps; ++j) {
            sim.step();
        }
        target.render(alpha);
    }

    EXPECT_EQ(sim.updates, 5U);

    ASSERT_EQ(target.alphas.size(), kFrames.size());
    for (std::size_t i = 0; i < target.alphas.size(); ++i) {
        SCOPED_TRACE(testing::Message() << "frame " << i + 1);
        EXPECT_DOUBLE_EQ(target.alphas[i], kFrames[i].alpha);
    }
}

TEST(FrameClockAdvance, TwoClocksFedTheSameSequenceReturnTheSameSteps) {
    constexpr std::array<std::chrono::nanoseconds, 4> kElapsed{6ms, 7ms, 32ms, 5ms};

    FrameClock a{kSandboxConfig};
    FrameClock b{kSandboxConfig};

    for (std::size_t i = 0; i < kElapsed.size(); ++i) {
        SCOPED_TRACE(testing::Message() << "frame " << i + 1);
        auto const steps_a = a.advance(kElapsed[i]).steps;
        auto const steps_b = b.advance(kElapsed[i]).steps;

        EXPECT_EQ(steps_a, steps_b);
    }
}

// The cap must be above the 25 steps that 250ms holds, so the clamp is what limits
// this call. With max_steps = 25, removing the clamp still returns 25.
TEST(FrameClockAdvance, ClampLimitsStepsWhenMaxStepsIsHigh) {
    constexpr FrameClockConfig kHighMaxStepsConfig{
        .fixed_step = 10ms,
        .max_elapsed = 250ms,
        .max_steps = 100,
    };
    FrameClock frame_clock{kHighMaxStepsConfig};

    auto const steps{frame_clock.advance(10s).steps};

    EXPECT_EQ(steps, 25U);
}

TEST(FrameClockAdvance, OneMoreNanosecondCompletesTheStep) {
    FrameClock frame_clock{kSandboxConfig};

    auto const first_steps{frame_clock.advance(9'999'999ns).steps};
    auto const second_steps{frame_clock.advance(1ns).steps};

    EXPECT_EQ(first_steps, 0U);
    EXPECT_EQ(second_steps, 1U);
}

// Every other case uses a 10ms step. With 10ms written into the step loop this
// returns 4 steps; with 10ms written into the alpha division, alpha is 0.8.
TEST(FrameClockAdvance, UsesTheConfiguredFixedStep) {
    constexpr FrameClockConfig kSixteenMsStepConfig{
        .fixed_step = 16ms,
        .max_elapsed = 250ms,
        .max_steps = 5,
    };
    FrameClock frame_clock{kSixteenMsStepConfig};

    auto const [steps, alpha] = frame_clock.advance(40ms);

    EXPECT_EQ(steps, 2U);
    EXPECT_DOUBLE_EQ(alpha, 0.5);
}
