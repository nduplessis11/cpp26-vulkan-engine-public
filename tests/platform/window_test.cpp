#include <cstdint>
#include <engine/platform/headless_window.hpp>
#include <engine/platform/window.hpp>
#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include <type_traits>

using engine::platform::Extent;
using engine::platform::HeadlessWindow;
using engine::platform::IWindow;
using engine::platform::WindowConfig;

// `window_a = window_b` through two IWindow& calls IWindow::operator=, which
// has no members to copy, so window_a keeps its old size and close state.
static_assert(!std::is_copy_assignable_v<IWindow> &&
                  !std::is_move_assignable_v<IWindow>,
              "Assigning through an IWindow& copies nothing. Keep IWindow's "
              "assignment operators protected.");

TEST(HeadlessWindow, ReportsConfiguredLogicalSize) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 1};

    auto const window_result{HeadlessWindow::create(kConfig)};

    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);
    IWindow const& window{**window_result};

    auto const logical_size{window.logical_size()};

    EXPECT_EQ(logical_size, kConfig.logical_size);
}

TEST(HeadlessWindow, RejectsZeroInitialWidth) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 0, .height = 600}, .scale = 1};

    auto const window_result{HeadlessWindow::create(kConfig)};
    EXPECT_FALSE(window_result.has_value());
}

TEST(HeadlessWindow, RejectsZeroInitialHeight) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 0}, .scale = 1};

    auto const window_result{HeadlessWindow::create(kConfig)};
    EXPECT_FALSE(window_result.has_value());
}

TEST(HeadlessWindow, RejectsZeroInitialScale) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 0};

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
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 2};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    IWindow const& window{**window_result};

    EXPECT_EQ(window.pixel_size(), (Extent{.width = 1600, .height = 1200}));
}

TEST(HeadlessWindow, ReportsConfiguredScale) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 2};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    IWindow const& window{**window_result};

    EXPECT_EQ(window.scale(), 2U);
}

TEST(HeadlessWindow, StartsWithoutCloseRequest) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 1U};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    IWindow const& window{**window_result};

    EXPECT_FALSE(window.should_close());
}

TEST(HeadlessWindow, AppliesCloseRequestOnPump) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 1U};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    HeadlessWindow& headless{**window_result};
    IWindow& window{**window_result};
    headless.post_close();
    EXPECT_FALSE(window.should_close());

    auto const& pump_result{window.pump()};
    ASSERT_TRUE(pump_result.has_value());
    EXPECT_TRUE(window.should_close());
}

TEST(HeadlessWindow, KeepsCloseRequestAfterAnotherPump) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 1U};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    HeadlessWindow& headless{**window_result};
    IWindow& window{headless};
    headless.post_close();

    auto const first_pump_result{window.pump()};
    ASSERT_TRUE(first_pump_result.has_value());
    EXPECT_TRUE(window.should_close());

    auto const second_pump_result{window.pump()};
    ASSERT_TRUE(second_pump_result.has_value());
    EXPECT_TRUE(window.should_close());
}

TEST(HeadlessWindow, AppliesResizeOnPump) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 1U};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    HeadlessWindow& headless{**window_result};
    IWindow& window{headless};

    auto const resize_result =
        headless.post_resize(Extent{.width = 1024, .height = 768}, 2U);
    ASSERT_TRUE(resize_result.has_value());

    EXPECT_EQ(window.logical_size(), (Extent{.width = 800, .height = 600}));
    EXPECT_EQ(window.pixel_size(), (Extent{.width = 800, .height = 600}));
    EXPECT_EQ(window.scale(), 1U);

    auto const pump_result{window.pump()};
    ASSERT_TRUE(pump_result.has_value());

    EXPECT_EQ(window.logical_size(), (Extent{.width = 1024, .height = 768}));
    EXPECT_EQ(window.pixel_size(), (Extent{.width = 2048, .height = 1536}));
    EXPECT_EQ(window.scale(), 2U);
}

struct InvalidResize {
    std::string_view name;
    Extent logical_size;
    std::uint32_t scale;
};

class RejectedResize : public testing::TestWithParam<InvalidResize> {};

INSTANTIATE_TEST_SUITE_P(
    HeadlessWindow, RejectedResize,
    testing::Values(
        InvalidResize{"ZeroScale", Extent{.width = 800, .height = 600}, 0U},
        InvalidResize{"PixelWidthOverflow",
                      Extent{.width = 2'147'483'648U, .height = 1}, 2U},
        InvalidResize{"PixelHeightOverflow",
                      Extent{.width = 1, .height = 2'147'483'648U}, 2U}),
    [](testing::TestParamInfo<InvalidResize> const& param_info) -> std::string {
        return std::string{param_info.param.name};
    });

TEST_P(RejectedResize, LeavesWindowUnchanged) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 1U};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    HeadlessWindow& headless{**window_result};
    IWindow& window{headless};

    auto const resize_result =
        headless.post_resize(GetParam().logical_size, GetParam().scale);
    EXPECT_FALSE(resize_result.has_value());

    auto const pump_result{window.pump()};
    ASSERT_TRUE(pump_result.has_value());

    EXPECT_EQ(window.logical_size(), (Extent{.width = 800, .height = 600}));
    EXPECT_EQ(window.pixel_size(), (Extent{.width = 800, .height = 600}));
    EXPECT_EQ(window.scale(), 1U);
}

TEST(HeadlessWindow, KeepsValidResizeAfterRejectedResize) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 1U};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    HeadlessWindow& headless{**window_result};
    IWindow& window{headless};

    auto const resize_result{
        headless.post_resize(Extent{.width = 1024, .height = 768}, 2U)};
    ASSERT_TRUE(resize_result.has_value());

    auto const bad_resize_result{
        headless.post_resize(Extent{.width = 640, .height = 480}, 0U)};
    ASSERT_FALSE(bad_resize_result.has_value());

    auto const pump_result{window.pump()};
    ASSERT_TRUE(pump_result.has_value());

    EXPECT_EQ(window.logical_size(), (Extent{.width = 1024, .height = 768}));
    EXPECT_EQ(window.pixel_size(), (Extent{.width = 2048, .height = 1536}));
    EXPECT_EQ(window.scale(), 2U);
}

TEST(HeadlessWindow, AppliesLastOfTwoResizes) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 1U};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    HeadlessWindow& headless{**window_result};
    IWindow& window{headless};

    auto const first_resize_result{
        headless.post_resize(Extent{.width = 1024, .height = 768}, 2U)};
    ASSERT_TRUE(first_resize_result.has_value());

    auto const second_resize_result{
        headless.post_resize(Extent{.width = 1280, .height = 720}, 1U)};
    ASSERT_TRUE(second_resize_result.has_value());

    auto const pump_result{window.pump()};
    ASSERT_TRUE(pump_result.has_value());

    EXPECT_EQ(window.logical_size(), (Extent{.width = 1280, .height = 720}));
    EXPECT_EQ(window.pixel_size(), (Extent{.width = 1280, .height = 720}));
    EXPECT_EQ(window.scale(), 1U);
}

TEST(HeadlessWindow, KeepsResizeAfterAnotherPump) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 1U};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    HeadlessWindow& headless{**window_result};
    IWindow& window{headless};

    auto const resize_result{
        headless.post_resize(Extent{.width = 1024, .height = 768}, 2U)};
    ASSERT_TRUE(resize_result.has_value());

    auto const first_pump_result{window.pump()};
    ASSERT_TRUE(first_pump_result.has_value());

    auto const second_pump_result{window.pump()};
    ASSERT_TRUE(second_pump_result.has_value());

    EXPECT_EQ(window.logical_size(), (Extent{.width = 1024, .height = 768}));
    EXPECT_EQ(window.pixel_size(), (Extent{.width = 2048, .height = 1536}));
    EXPECT_EQ(window.scale(), 2U);
    EXPECT_FALSE(window.should_close());
}

TEST(HeadlessWindow, WindowsKeepSeparateState) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 1U};

    auto const first_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(first_result.has_value());
    ASSERT_NE(first_result->get(), nullptr);

    auto const second_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(second_result.has_value());
    ASSERT_NE(second_result->get(), nullptr);

    HeadlessWindow& first_headless{**first_result};
    HeadlessWindow& second_headless{**second_result};
    IWindow& first{first_headless};
    IWindow& second{second_headless};

    auto const resize_result{
        first_headless.post_resize(Extent{.width = 1024, .height = 768}, 2U)};
    ASSERT_TRUE(resize_result.has_value());

    auto const second_resize_pump_result{second.pump()};
    ASSERT_TRUE(second_resize_pump_result.has_value());
    EXPECT_EQ(second.logical_size(), (Extent{.width = 800, .height = 600}));
    EXPECT_EQ(second.scale(), 1U);

    second_headless.post_close();

    auto const first_pump_result{first.pump()};
    ASSERT_TRUE(first_pump_result.has_value());
    EXPECT_EQ(first.logical_size(), (Extent{.width = 1024, .height = 768}));
    EXPECT_EQ(first.scale(), 2U);
    EXPECT_FALSE(first.should_close());

    auto const second_close_pump_result{second.pump()};
    ASSERT_TRUE(second_close_pump_result.has_value());
    EXPECT_TRUE(second.should_close());
    EXPECT_EQ(second.logical_size(), (Extent{.width = 800, .height = 600}));
    EXPECT_EQ(second.scale(), 1U);
}

TEST(HeadlessWindow, AppliesCloseAndResizeInOnePump) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 1U};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    HeadlessWindow& headless{**window_result};
    IWindow& window{headless};

    headless.post_close();
    auto const resize_result{
        headless.post_resize(Extent{.width = 1024, .height = 768}, 2U)};
    ASSERT_TRUE(resize_result.has_value());

    auto const pump_result{window.pump()};
    ASSERT_TRUE(pump_result.has_value());

    EXPECT_TRUE(window.should_close());
    EXPECT_EQ(window.logical_size(), (Extent{.width = 1024, .height = 768}));
    EXPECT_EQ(window.pixel_size(), (Extent{.width = 2048, .height = 1536}));
    EXPECT_EQ(window.scale(), 2U);
}

TEST(HeadlessWindow, HasNoNativeHandle) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 1U};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    IWindow const& window{**window_result};

    auto const handle = window.native_handle();
    EXPECT_FALSE(handle.has_value());
}

TEST(HeadlessWindow, AppliesZeroSizeResizes) {
    constexpr WindowConfig kConfig{
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 1U};

    auto const window_result{HeadlessWindow::create(kConfig)};
    ASSERT_TRUE(window_result.has_value());
    ASSERT_NE(window_result->get(), nullptr);

    HeadlessWindow& headless{**window_result};
    IWindow& window{headless};

    auto const zero_width{
        headless.post_resize(Extent{.width = 0, .height = 600}, 1U)};
    ASSERT_TRUE(zero_width.has_value());
    auto const pump_zero_width{window.pump()};
    ASSERT_TRUE(pump_zero_width.has_value());
    EXPECT_EQ(window.logical_size(), (Extent{.width = 0, .height = 600}));
    EXPECT_EQ(window.pixel_size(), (Extent{.width = 0, .height = 600}));

    auto const zero_height{
        headless.post_resize(Extent{.width = 800, .height = 0}, 1U)};
    ASSERT_TRUE(zero_height.has_value());
    auto const pump_zero_height{window.pump()};
    ASSERT_TRUE(pump_zero_height.has_value());
    EXPECT_EQ(window.logical_size(), (Extent{.width = 800, .height = 0}));
    EXPECT_EQ(window.pixel_size(), (Extent{.width = 800, .height = 0}));

    auto const resize_result{
        headless.post_resize(Extent{.width = 1024, .height = 768}, 2U)};
    ASSERT_TRUE(resize_result.has_value());
    auto const pump_result{window.pump()};
    ASSERT_TRUE(pump_result.has_value());
    EXPECT_EQ(window.logical_size(), (Extent{.width = 1024, .height = 768}));
    EXPECT_EQ(window.pixel_size(), (Extent{.width = 2048, .height = 1536}));

    EXPECT_FALSE(window.native_handle().has_value());
}
