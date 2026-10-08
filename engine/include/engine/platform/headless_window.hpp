#pragma once

#include <cstdint>
#include <engine/platform/window.hpp>
#include <expected>
#include <limits>
#include <memory>
#include <optional>

namespace engine::platform {

class HeadlessWindow : public Window {
  public:
    static auto create(WindowConfig const config)
        -> std::expected<std::unique_ptr<HeadlessWindow>, WindowError> {
        if (!is_valid(config)) {
            return std::unexpected{WindowError{}};
        }

        return std::unique_ptr<HeadlessWindow>{new HeadlessWindow{config}};
    }

    auto logical_size() const -> Extent override {
        return logical_size_;
    }

    auto pixel_size() const -> Extent override {
        return Extent{.width = logical_size_.width * scale_, .height = logical_size_.height * scale_};
    }

    auto scale() const -> std::uint32_t override {
        return scale_;
    }

    auto should_close() const -> bool override {
        return should_close_;
    }

    auto post_close() -> void {
        pending_close_ = true;
    }

    auto post_resize(Extent logical_size, std::uint32_t scale)
        -> std::expected<void, WindowError> {
        auto const config{WindowConfig{.logical_size = logical_size, .scale = scale}};
        if (!is_valid(config)) {
            return std::unexpected{WindowError{}};
        }

        pending_resize_ = config;

        return {};
    }

    auto pump() -> std::expected<void, WindowError> override {
        if (pending_close_) {
            should_close_ = true;
        }
        pending_close_ = false;

        if (pending_resize_) {
            logical_size_ = pending_resize_->logical_size;
            scale_ = pending_resize_->scale;
            pending_resize_.reset();
        }

        return {};
    }

  private:
    static auto is_valid(WindowConfig config) -> bool {
        if (config.logical_size.width == 0 || config.logical_size.height == 0 ||
            config.scale == 0) {
            return false;
        }

        auto const pixel_width{
            static_cast<std::uint64_t>(config.logical_size.width) * config.scale};
        auto const pixel_height{
            static_cast<std::uint64_t>(config.logical_size.height) * config.scale};

        if (pixel_width > std::numeric_limits<std::uint32_t>::max() ||
            pixel_height > std::numeric_limits<std::uint32_t>::max()) {
            return false;
        }

        return true;
    }

    explicit HeadlessWindow(WindowConfig const config)
        : logical_size_{config.logical_size}, scale_{config.scale} {}

    Extent logical_size_;
    std::uint32_t scale_;
    bool should_close_{false};
    bool pending_close_{false};
    std::optional<WindowConfig> pending_resize_;
};

} // namespace engine::platform
