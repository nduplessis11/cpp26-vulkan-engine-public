#pragma once

#include <cstdint>
#include <expected>

namespace engine::core {
class Logger;
}
namespace engine::platform {
enum class WindowError : std::uint8_t;
}

namespace engine::platform::wayland {

struct WaylandGlobalVersions {
    std::uint32_t compositor;
    std::uint32_t xdg_wm_base;
};

auto query_wayland_globals(engine::core::Logger const& logger)
    -> std::expected<WaylandGlobalVersions, WindowError>;

} // namespace engine::platform::wayland
