#pragma once
#include <string_view>

namespace engine::core {

[[nodiscard]] constexpr std::string_view scaffold_name() noexcept {
    return "C++26 Vulkan Engine learning scaffold";
}

[[nodiscard]] constexpr std::string_view scaffold_version() noexcept {
    return "0.0.0-learning";
}

} // namespace engine::core
