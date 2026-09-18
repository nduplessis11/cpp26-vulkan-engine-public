#include <engine/platform/headless_window.hpp>
#include <engine/platform/window.hpp>
#include <gtest/gtest.h>

using engine::platform::Extent;
using engine::platform::HeadlessWindow;
using engine::platform::Window;
using engine::platform::WindowConfig;

TEST(HeadlessWindow, ReportsConfiguredLogicalSize) {
    WindowConfig const config{Extent{800, 600}, 1};

    auto const window_result{HeadlessWindow::create(config)};
    Window& window{**window_result};
    auto const logical_size{window.logical_size()};

    EXPECT_EQ(logical_size, config.logical_size);
}
