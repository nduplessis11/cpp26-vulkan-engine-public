#include "wayland_connection.hpp"

#include "engine/platform/wayland.hpp"
#include "registry_globals.hpp"

#include <cstdint>
#include <engine/core/logger.hpp>
#include <engine/platform/window.hpp>
#include <expected>
#include <memory>
#include <string_view>
#include <utility>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>

namespace engine::platform::wayland {

using engine::core::Category;
using engine::core::Logger;
using engine::core::Severity;

namespace {

constexpr Category kWayland{"wayland"};

auto on_global(void* data, wl_registry*, std::uint32_t const name,
               char const* interface, std::uint32_t const version) -> void {
    auto* found{static_cast<FoundGlobals*>(data)};
    record_global(*found, name, std::string_view{interface}, version);
}

auto on_global_remove(void*, wl_registry*, std::uint32_t) -> void {}

auto on_ping(void*, xdg_wm_base* wm_base, std::uint32_t serial) -> void {
    xdg_wm_base_pong(wm_base, serial);
}

constexpr wl_registry_listener kRegistryListener{
    .global = on_global, .global_remove = on_global_remove};

constexpr xdg_wm_base_listener kWmBaseListener{.ping = on_ping};

} // namespace

auto DisplayDeleter::operator()(wl_display* display) const noexcept -> void {
    wl_display_flush(display);
    wl_display_disconnect(display);
}

auto RegistryDeleter::operator()(wl_registry* registry) const noexcept -> void {
    wl_registry_destroy(registry);
}

auto CompositorDeleter::operator()(wl_compositor* compositor) const noexcept
    -> void {
    wl_compositor_destroy(compositor);
}

auto WmBaseDeleter::operator()(xdg_wm_base* wm_base) const noexcept -> void {
    xdg_wm_base_destroy(wm_base);
}

auto WaylandConnection::create(char const* socket_name, Logger const& logger)
    -> std::expected<std::unique_ptr<WaylandConnection>, WindowError> {

    DisplayPtr display{wl_display_connect(socket_name)};
    if (display == nullptr) {
        return std::unexpected{WindowError::NoDisplay};
    }

    auto connection{std::unique_ptr<WaylandConnection>{
        new WaylandConnection{std::move(display)}}};

    connection->registry_.reset(
        wl_display_get_registry(connection->display_.get()));
    if (connection->registry_ == nullptr) {
        return std::unexpected{WindowError::ProxyCreationFailed};
    }

    wl_registry_add_listener(connection->registry_.get(), &kRegistryListener,
                             &connection->found_);

    if (wl_display_roundtrip(connection->display_.get()) < 0) {
        logger.log(Severity::Error, kWayland,
                   "Wayland display connection failed.");
        return std::unexpected{WindowError::ConnectionFailed};
    }

    auto const globals{choose_globals(connection->found_)};
    if (!globals) {
        return std::unexpected{globals.error()};
    }
    connection->bound_ = *globals;

    logger.log(Severity::Debug, kWayland, "compositor: {}v{}",
               globals->compositor.name, globals->compositor.version);

    logger.log(Severity::Debug, kWayland, "wm_base: {}v{}",
               globals->wm_base.name, globals->wm_base.version);

    connection->compositor_.reset(static_cast<wl_compositor*>(wl_registry_bind(
        connection->registry_.get(), globals->compositor.name,
        &wl_compositor_interface, globals->compositor.version)));
    if (connection->compositor_ == nullptr) {
        return std::unexpected{WindowError::ProxyCreationFailed};
    }

    connection->wm_base_.reset(static_cast<xdg_wm_base*>(
        wl_registry_bind(connection->registry_.get(), globals->wm_base.name,
                         &xdg_wm_base_interface, globals->wm_base.version)));
    if (connection->wm_base_ == nullptr) {
        return std::unexpected{WindowError::ProxyCreationFailed};
    }

    xdg_wm_base_add_listener(connection->wm_base_.get(), &kWmBaseListener,
                             nullptr);

    if (wl_display_roundtrip(connection->display_.get()) < 0) {
        logger.log(Severity::Error, kWayland,
                   "Binding compositor/wm_base failed.");
        return std::unexpected{WindowError::ConnectionFailed};
    }

    return connection;
}

auto WaylandConnection::bound_globals() const -> RequiredGlobals {
    return bound_;
}

auto query_wayland_globals(Logger const& logger)
    -> std::expected<WaylandGlobalVersions, WindowError> {

    auto const connection{WaylandConnection::create(nullptr, logger)};
    if (!connection) {
        return std::unexpected{connection.error()};
    }
    auto const globals{(*connection)->bound_globals()};
    return WaylandGlobalVersions{
        .compositor = globals.compositor.version,
        .xdg_wm_base = globals.wm_base.version,
    };
}

} // namespace engine::platform::wayland
