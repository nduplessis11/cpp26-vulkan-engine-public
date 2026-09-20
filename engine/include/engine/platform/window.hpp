#pragma once

#include <cstdint>
#include <expected>
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
    virtual auto pixel_size() const -> Extent = 0;
    virtual auto scale() const -> std::uint32_t = 0;
    virtual auto should_close() const -> bool = 0;
    virtual auto pump() -> std::expected<void, WindowError> = 0;
};

} // namespace engine::platform
