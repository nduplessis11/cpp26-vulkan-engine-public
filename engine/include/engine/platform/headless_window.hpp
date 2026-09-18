#pragma once

#include <engine/platform/window.hpp>
#include <expected>
#include <memory>

namespace engine::platform {

class HeadlessWindow : public Window {
  public:
    static auto create(WindowConfig config)
        -> std::expected<std::unique_ptr<HeadlessWindow>, WindowError> {
        return std::unique_ptr<HeadlessWindow>{new HeadlessWindow{config}};
    }

    auto logical_size() const -> Extent override {
        return logical_size_;
    }

  private:
    explicit HeadlessWindow(WindowConfig const config) : logical_size_{config.logical_size} {}
    Extent logical_size_;
};

} // namespace engine::platform
