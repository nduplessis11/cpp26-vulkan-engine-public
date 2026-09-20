#include <engine/platform/headless_window.hpp>
#include <engine/platform/window.hpp>
#include <gtest/gtest.h>

using engine::platform::Extent;
using engine::platform::HeadlessWindow;
using engine::platform::Window;
using engine::platform::WindowConfig;

TEST(HeadlessWindow, ReportsConfiguredLogicalSize) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600},
        .scale = 1};

    auto const window_result{HeadlessWindow::create(kConfig)};

    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);
    Window const& window{**window_result};

    auto const logical_size{window.logical_size()};

    EXPECT_EQ(logical_size, kConfig.logical_size);
}

TEST(HeadlessWindow, RejectsZeroInitialWidth) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 0, .height = 600},
        .scale = 1};

    auto const window_result{HeadlessWindow::create(kConfig)};
    EXPECT_FALSE(window_result.has_value());
}

TEST(HeadlessWindow, RejectsZeroInitialHeight) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 0},
        .scale = 1};

    auto const window_result{HeadlessWindow::create(kConfig)};
    EXPECT_FALSE(window_result.has_value());
}

TEST(HeadlessWindow, RejectsZeroInitialScale) {
    constexpr WindowConfig kConfig{.logical_size = Extent{.width = 800, .height = 600},
                                   .scale = 0};

    auto const window_result{HeadlessWindow::create(kConfig)};
    EXPECT_FALSE(window_result.has_value());
}

TEST(HeadlessWindow, RejectsInitialPixelWidthOverflow) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 2'147'483'648U, .height = 1},
        .scale = 2};

    auto const window_result{HeadlessWindow::create(kConfig)};
    EXPECT_FALSE(window_result.has_value());
}

TEST(HeadlessWindow, RejectsInitialPixelHeightOverflow) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 1, .height = 2'147'483'648U},
        .scale = 2};

    auto const window_result{HeadlessWindow::create(kConfig)};
    EXPECT_FALSE(window_result.has_value());
}

TEST(HeadlessWindow, ReportsScaledPixelSize) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600},
        .scale = 2};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    Window const& window{**window_result};

    EXPECT_EQ(window.pixel_size(), (Extent{.width = 1600, .height = 1200}));
}

TEST(HeadlessWindow, ReportsConfiguredScale) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600},
        .scale = 2};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    Window const& window{**window_result};

    EXPECT_EQ(window.scale(), 2U);
}

TEST(HeadlessWindow, StartsWithoutCloseRequest) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600},
        .scale = 1U};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    Window const& window{**window_result};

    EXPECT_FALSE(window.should_close());
}

TEST(HeadlessWindow, AppliesCloseRequestOnPump) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600},
        .scale = 1U};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    HeadlessWindow& headless{**window_result};
    Window& window{**window_result};
    headless.post_close();
    EXPECT_FALSE(window.should_close());

    auto const& pump_result{window.pump()};
    ASSERT_TRUE(pump_result.has_value());
    EXPECT_TRUE(window.should_close());
}

TEST(HeadlessWindow, KeepsCloseRequestAfterAnotherPump) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600},
        .scale = 1U};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    HeadlessWindow& headless{**window_result};
    Window& window{headless};
    headless.post_close();

    auto const first_pump_result{window.pump()};
    ASSERT_TRUE(first_pump_result.has_value());
    EXPECT_TRUE(window.should_close());

    auto const second_pump_result{window.pump()};
    ASSERT_TRUE(second_pump_result.has_value());
    EXPECT_TRUE(window.should_close());
}
