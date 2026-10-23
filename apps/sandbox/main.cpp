#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <engine/core/frame_clock.hpp>
#include <engine/core/logger.hpp>
#include <engine/core/scaffold_info.hpp>
#include <engine/platform/headless_window.hpp>
#include <engine/platform/wayland.hpp>
#include <engine/platform/window.hpp>
#include <exception>
#include <memory>
#include <thread>
#include <utility>

#if ENGINE_HAS_VULKAN_SDK
#include <vulkan/vulkan_core.h>
#endif

namespace {

using engine::core::Category;
using engine::core::FrameClock;
using engine::core::FrameClockConfig;
using engine::core::Logger;
using engine::core::Severity;
using engine::core::StderrSink;
using engine::platform::Extent;
using engine::platform::HeadlessWindow;
using engine::platform::IWindow;
using engine::platform::WindowConfig;
using engine::platform::WindowError;
using engine::platform::wayland::query_wayland_globals;

using namespace std::chrono_literals;

constexpr Category kSandbox{"sandbox"};
constexpr Category kVulkan{"vulkan"};
constexpr Category kWayland{"wayland"};
constexpr Category kWindow{"window"};

constexpr FrameClockConfig kSandboxConfig{
    .fixed_step = 10ms,
    .max_elapsed = 250ms,
    .max_steps = 5,
};
constexpr std::size_t kFrameCount = 600;

auto run() -> int {
    StderrSink sink;
    Logger logger{sink, Severity::Debug};
    FrameClock clock{kSandboxConfig};

    logger.log(Severity::Info, kSandbox, engine::core::scaffold_name());
    logger.log(Severity::Info, kSandbox, "scaffold version {}",
               engine::core::scaffold_version());
    logger.log(Severity::Debug, kSandbox, "__cplusplus {}", __cplusplus);

#if ENGINE_HAS_VULKAN_SDK
    logger.log(Severity::Info, kVulkan, "header {}.{}.{}",
               VK_API_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE),
               VK_API_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE),
               VK_API_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE));
#else
    logger.log(Severity::Warn, kVulkan,
               "1.4 SDK not found; scaffold-only configuration");
#endif

#if ENGINE_HAS_WAYLAND
    auto const wayland_globals{query_wayland_globals(logger)};
    if (!wayland_globals) {
        if (wayland_globals.error() == WindowError::NoDisplay) {
            logger.log(Severity::Info, kWayland,
                       "no Wayland display; running headless");
        } else {
            logger.log(Severity::Error, kWayland,
                       "Error fetching wayland_globals: {}",
                       engine::platform::describe(wayland_globals.error()));
            return EXIT_FAILURE;
        }
    } else {
        logger.log(Severity::Info, kWayland, "Compositor: {}",
                   wayland_globals->compositor);
        logger.log(Severity::Info, kWayland, "WM Base: {}",
                   wayland_globals->xdg_wm_base);
    }
#endif

    auto constexpr config{WindowConfig{
        .logical_size = Extent{.width = 800, .height = 600}, .scale = 1U}};
    auto window_result = HeadlessWindow::create(config);
    if (!window_result.has_value()) {
        logger.log(Severity::Error, kWindow, "Window creation failure");
        return EXIT_FAILURE;
    }

    std::unique_ptr<IWindow> window = std::move(*window_result);

    auto previous{std::chrono::steady_clock::now()};
    std::size_t frame{0};
    std::size_t updates{0};

    while (frame < kFrameCount) {
        auto const pump_result{window->pump()};
        if (!pump_result.has_value()) {
            logger.log(Severity::Error, kWindow, "Window message pump failure");
            return EXIT_FAILURE;
        }

        if (window->should_close()) {
            break;
        }

        auto const now{std::chrono::steady_clock::now()};
        auto const budget{clock.advance(now - previous)};
        previous = now;

        for (std::size_t step = 0; step < budget.steps; ++step) {
            ++updates;
        }

        // Without a sleep, all 600 frames finish in less than one 10ms step
        // and no update runs. Sleeping 1ms per frame makes the run take at
        // least 600ms.
        std::this_thread::sleep_for(1ms);
        ++frame;
    }
    logger.log(Severity::Info, kSandbox, "Updates: {}, Frame: {}", updates,
               frame);

    return EXIT_SUCCESS;
}

} // namespace

auto main() -> int {
    try {
        return run();
    } catch (std::exception const& error) {
        // Report directly; formatting or allocating a message could fail again.
        std::fputs("[error] sandbox: ", stderr);
        std::fputs(error.what(), stderr);
        std::fputs("\n", stderr);
        return EXIT_FAILURE;
    }
}
