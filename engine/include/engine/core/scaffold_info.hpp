#pragma once
#include <string_view>

namespace engine::core {

[[nodiscard]] constexpr auto scaffold_name() noexcept -> std::string_view {
    return "C++26 Vulkan Engine learning scaffold";
}

[[nodiscard]] constexpr auto scaffold_version() noexcept -> std::string_view {
    return "0.0.0-learning";
}

} // namespace engine::core
