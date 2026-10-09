#pragma once

#include <cstdint>
#include <variant>

namespace engine::platform {

struct WaylandDisplay {
    std::uintptr_t value;
};

struct WaylandSurface {
    std::uintptr_t value;
};

struct WaylandWindowHandle {
    WaylandDisplay display;
    WaylandSurface surface;
};

struct Win32Instance {
    std::uintptr_t value;
};

struct Win32Window {
    std::uintptr_t value;
};

struct Win32WindowHandle {
    Win32Instance instance;
    Win32Window window;
};

struct XcbConnection {
    std::uintptr_t value;
};

struct XcbWindow {
    std::uint32_t value;
};

struct XcbWindowHandle {
    XcbConnection connection;
    XcbWindow window;
};

using NativeWindowHandle =
    std::variant<WaylandWindowHandle, Win32WindowHandle, XcbWindowHandle>;

} // namespace engine::platform
