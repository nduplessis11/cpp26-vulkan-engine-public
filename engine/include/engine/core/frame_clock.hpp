#pragma once

#include <algorithm>
#include <chrono>
#include <cstddef> // IWYU pragma: keep

namespace engine::core {

struct FrameClockConfig {
    std::chrono::nanoseconds fixed_step;
    std::chrono::nanoseconds max_elapsed;
    std::size_t max_steps;
};

struct FrameBudget {
    std::size_t steps;
    double alpha;
};

class FrameClock {
  public:
    // clang-format 22 has no rule for contract clauses and joins them onto one line,
    // where a reader cannot scan the conditions. Keep one clause per line by hand.
    // clang-format off
    explicit FrameClock(FrameClockConfig const& config)
        pre(config.fixed_step > std::chrono::nanoseconds{0})
        pre(config.max_steps > 0)
        : config_(config) {}

    [[nodiscard]]
    auto advance(std::chrono::nanoseconds const elapsed) -> FrameBudget
        post(accumulator_ >= std::chrono::nanoseconds{0})
        post(accumulator_ < config_.fixed_step) {
        // clang-format on
        auto const frame_time =
            std::clamp(elapsed, std::chrono::nanoseconds{0}, config_.max_elapsed);
        std::size_t steps = 0;
        accumulator_ += frame_time;
        while (accumulator_ >= config_.fixed_step && steps < config_.max_steps) {
            accumulator_ -= config_.fixed_step;
            steps++;
        }

        // If max_steps capped the frame, discard any full steps still left over.
        // Keep only the fractional remainder used to calculate alpha.
        accumulator_ %= config_.fixed_step;

        auto const alpha{std::chrono::duration<double>{accumulator_} /
                         std::chrono::duration<double>{config_.fixed_step}};

        return FrameBudget{.steps = steps, .alpha = alpha};
    }

  private:
    FrameClockConfig config_;
    std::chrono::nanoseconds accumulator_{0};
};

} // namespace engine::core
