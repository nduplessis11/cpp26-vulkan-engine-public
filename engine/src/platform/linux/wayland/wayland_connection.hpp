#pragma once

#include "registry_globals.hpp"

#include <engine/platform/window.hpp>
#include <expected>
#include <memory>
#include <utility>

struct wl_display;
struct wl_registry;
struct wl_compositor;
struct xdg_wm_base;

namespace engine::core {
class Logger;
}

namespace engine::platform::wayland {

struct DisplayDeleter {
    auto operator()(wl_display* display) const noexcept -> void;
};
using DisplayPtr = std::unique_ptr<wl_display, DisplayDeleter>;

struct RegistryDeleter {
    auto operator()(wl_registry* registry) const noexcept -> void;
};
using RegistryPtr = std::unique_ptr<wl_registry, RegistryDeleter>;

struct CompositorDeleter {
    auto operator()(wl_compositor* compositor) const noexcept -> void;
};
using CompositorPtr = std::unique_ptr<wl_compositor, CompositorDeleter>;

struct WmBaseDeleter {
    auto operator()(xdg_wm_base* wm_base) const noexcept -> void;
};
using WmBasePtr = std::unique_ptr<xdg_wm_base, WmBaseDeleter>;

class WaylandConnection {
  public:
    static auto create(char const* socket_name,
                       engine::core::Logger const& logger)
        -> std::expected<std::unique_ptr<WaylandConnection>, WindowError>;

    auto bound_globals() const -> RequiredGlobals;

    WaylandConnection(WaylandConnection const&) = delete;
    auto operator=(WaylandConnection const&) -> WaylandConnection& = delete;
    WaylandConnection(WaylandConnection&&) = delete;
    auto operator=(WaylandConnection&&) -> WaylandConnection& = delete;
    ~WaylandConnection() = default;

  private:
    explicit WaylandConnection(DisplayPtr display)
        : display_{std::move(display)} {}
    DisplayPtr display_;
    FoundGlobals found_;
    RegistryPtr registry_;
    RequiredGlobals bound_{};
    CompositorPtr compositor_;
    WmBasePtr wm_base_;
};

} // namespace engine::platform::wayland
