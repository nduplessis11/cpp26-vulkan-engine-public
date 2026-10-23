#include <engine/core/logger.hpp>
#include <engine/platform/wayland.hpp>
#include <engine/platform/window.hpp>
#include <expected>
#include <gtest/gtest.h>
#include <platform/linux/wayland/registry_globals.hpp>
#include <platform/linux/wayland/wayland_connection.hpp>
#include <stdlib.h>

using engine::core::CaptureSink;
using engine::core::Logger;
using engine::core::Severity;
using engine::platform::WindowError;
using engine::platform::wayland::choose_globals;
using engine::platform::wayland::FoundGlobals;
using engine::platform::wayland::GlobalEntry;
using engine::platform::wayland::query_wayland_globals;
using engine::platform::wayland::record_global;
using engine::platform::wayland::WaylandConnection;

TEST(RegistryGlobals, RecordsCompositorAndWmBase) {
    FoundGlobals found;
    record_global(found, 1, "wl_shm", 1);
    EXPECT_EQ(found, (FoundGlobals{}));

    record_global(found, 3, "wl_compositor", 6);
    EXPECT_EQ(found.compositor, (GlobalEntry{3, 6}));

    record_global(found, 37, "xdg_wm_base", 7);
    EXPECT_EQ(found.wm_base, (GlobalEntry{37, 7}));
}

// A prefix or substring match would record these as the required globals.
TEST(RegistryGlobals, IgnoresOtherInterfaces) {
    FoundGlobals found;
    record_global(found, 4, "wl_compositor_x", 6);
    record_global(found, 38, "xdg_wm_base_x", 7);

    EXPECT_EQ(found, (FoundGlobals{}));
}

TEST(RegistryGlobals, ReportsMissingCompositor) {
    FoundGlobals found;
    record_global(found, 38, "xdg_wm_base", 7);

    auto const result{choose_globals(found)};
    ASSERT_FALSE(result);
    EXPECT_EQ(result, std::unexpected{WindowError::MissingCompositor});
}

TEST(RegistryGlobals, ReportsMissingXdgWmBase) {
    FoundGlobals found;
    record_global(found, 3, "wl_compositor", 6);

    auto const result{choose_globals(found)};
    ASSERT_FALSE(result);
    EXPECT_EQ(result, std::unexpected{WindowError::MissingXdgWmBase});
}

// Hyprland offers wl_compositor v6 and xdg_wm_base v7. The engine's listeners
// handle events up to v5 and v2.
TEST(RegistryGlobals, BindsSupportedVersionWhenOfferedNewer) {
    FoundGlobals found;
    record_global(found, 3, "wl_compositor", 6);
    record_global(found, 37, "xdg_wm_base", 7);

    auto const result{choose_globals(found)};
    ASSERT_TRUE(result);
    EXPECT_EQ(result->compositor, (GlobalEntry{3, 5}));
    EXPECT_EQ(result->wm_base, (GlobalEntry{37, 2}));
}

// Binding a version newer than the compositor offers is a protocol error.
TEST(RegistryGlobals, BindsOfferedVersionWhenOlder) {
    FoundGlobals found;
    record_global(found, 3, "wl_compositor", 4);
    record_global(found, 37, "xdg_wm_base", 1);

    auto const result{choose_globals(found)};
    ASSERT_TRUE(result);
    EXPECT_EQ(result->compositor, (GlobalEntry{3, 4}));
    EXPECT_EQ(result->wm_base, (GlobalEntry{37, 1}));
}

TEST(WaylandConnection, ReportsNoDisplayForMissingSocket) {
    CaptureSink sink;
    Logger logger{sink, Severity::Debug};

    auto const connection{WaylandConnection::create("does not exist", logger)};
    ASSERT_FALSE(connection);
    EXPECT_EQ(connection, std::unexpected{WindowError::NoDisplay});
}

// Set WAYLAND_DISPLAY to a socket name that does not exist and unset
// WAYLAND_SOCKET, so the result is the same in Docker and on the Hyprland
// desktop.
TEST(QueryWaylandGlobals, ReportsNoDisplayForMissingDefaultSocket) {
    ::setenv("WAYLAND_DISPLAY", "no-such-socket", 1);
    ::unsetenv("WAYLAND_SOCKET");

    CaptureSink sink;
    Logger logger{sink, Severity::Debug};

    auto const result{query_wayland_globals(logger)};

    ASSERT_FALSE(result);
    EXPECT_EQ(result, std::unexpected{WindowError::NoDisplay});
}
