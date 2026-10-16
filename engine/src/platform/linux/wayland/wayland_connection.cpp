#include "wayland_connection.hpp"

#include "engine/platform/window.hpp"

#include <expected>
#include <memory>
#include <utility>
#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>

namespace engine::platform::wayland {

auto DisplayDeleter::operator()(wl_display* display) const noexcept -> void {
    wl_display_flush(display);
    wl_display_disconnect(display);
}

auto WaylandConnection::create(char const* socket_name)
    -> std::expected<std::unique_ptr<WaylandConnection>, WindowError> {

    DisplayPtr display{wl_display_connect(socket_name)};
    if (display == nullptr) {
        return std::unexpected{WindowError::NoDisplay};
    }

    return std::unique_ptr<WaylandConnection>{
        new WaylandConnection{std::move(display)}};
}

} // namespace engine::platform::wayland
