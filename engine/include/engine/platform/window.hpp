#pragma once

#include <cstdint>
#include <engine/platform/native_window_handle.hpp>
#include <expected>
#include <optional>
#include <string_view>

namespace engine::platform {

struct Extent {
    std::uint32_t width;
    std::uint32_t height;

    constexpr auto operator==(Extent const& other) const -> bool = default;
};

struct WindowConfig {
    Extent logical_size;
    std::uint32_t scale;
};

// REFACTOR?: This leaks implementation details.
enum class WindowError : std::uint8_t {
    InvalidConfig,
    NoDisplay,
    ConnectionFailed,
    MissingCompositor,
    MissingXdgWmBase,
    ProxyCreationFailed,
};

constexpr auto describe(WindowError error) -> std::string_view {
    switch (error) {
    case WindowError::InvalidConfig:
        return "Invalid window config";
        break;
    case WindowError::NoDisplay:
        return "No display";
        break;
    case WindowError::ConnectionFailed:
        return "Connection failed";
        break;
    case WindowError::MissingCompositor:
        return "wl_compositor is missing";
        break;
    case WindowError::MissingXdgWmBase:
        return "xdg_wm_base is missing";
        break;
    case WindowError::ProxyCreationFailed:
        return "Proxy creation failed";
        break;
    }
}

struct IWindow {
    virtual ~IWindow() = default;
    virtual auto logical_size() const -> Extent = 0;
    virtual auto pixel_size() const -> Extent = 0;
    virtual auto scale() const -> std::uint32_t = 0;
    virtual auto should_close() const -> bool = 0;
    virtual auto pump() -> std::expected<void, WindowError> = 0;
    virtual auto native_handle() const -> std::optional<NativeWindowHandle> = 0;

  protected:
    IWindow() = default;
    IWindow(IWindow const&) = default;
    IWindow(IWindow&&) = default;
    auto operator=(IWindow const&) -> IWindow& = default;
    auto operator=(IWindow&&) -> IWindow& = default;
};

} // namespace engine::platform
