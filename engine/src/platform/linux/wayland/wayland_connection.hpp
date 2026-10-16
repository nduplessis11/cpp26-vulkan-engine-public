#pragma once

#include <engine/platform/window.hpp>
#include <expected>
#include <memory>
#include <utility>

struct wl_display;

namespace engine::platform::wayland {

struct DisplayDeleter {
    auto operator()(wl_display* display) const noexcept -> void;
};

using DisplayPtr = std::unique_ptr<wl_display, DisplayDeleter>;

class WaylandConnection {
  public:
    static auto create(char const* socket_name)
        -> std::expected<std::unique_ptr<WaylandConnection>, WindowError>;

    WaylandConnection(WaylandConnection const&) = delete;
    auto operator=(WaylandConnection const&) -> WaylandConnection& = delete;
    WaylandConnection(WaylandConnection&&) = delete;
    auto operator=(WaylandConnection&&) -> WaylandConnection& = delete;
    ~WaylandConnection() = default;

  private:
    explicit WaylandConnection(DisplayPtr display)
        : display_{std::move(display)} {}
    DisplayPtr display_;
};

} // namespace engine::platform::wayland
