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
        std::chrono::nanoseconds remainder{elapsed};
        while (remainder >= config_.fixed_step) {
            remainder -= config_.fixed_step;
            steps++;
        }

        return FrameBudget{.steps = steps, .alpha = 0};
    }

  private:
    FrameClockConfig config_;
};

} // namespace engine::core
