#pragma once

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
    explicit FrameClock(FrameClockConfig const& config) : config_(config) {}

    [[nodiscard]]
    auto advance(std::chrono::nanoseconds const elapsed) -> FrameBudget {
        std::size_t steps = 0;
        accumulator_ += elapsed;
        while (accumulator_ >= config_.fixed_step) {
            accumulator_ -= config_.fixed_step;
            steps++;
        }

        auto const alpha{std::chrono::duration<double>{accumulator_} /
                         std::chrono::duration<double>{config_.fixed_step}};

        return FrameBudget{.steps = steps, .alpha = alpha};
    }

  private:
    FrameClockConfig config_;
    std::chrono::nanoseconds accumulator_{0};
};

} // namespace engine::core
