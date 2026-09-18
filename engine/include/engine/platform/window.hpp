#pragma once

#include <cstdint>
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

struct WindowError {};

struct Window {
    virtual ~Window() = default;
    virtual auto logical_size() const -> Extent = 0;
};

} // namespace engine::platform
