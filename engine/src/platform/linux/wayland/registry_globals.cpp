#include "registry_globals.hpp"

#include "engine/platform/window.hpp"

#include <algorithm>
#include <cstdint>
#include <expected>
#include <string_view>

namespace engine::platform::wayland {

namespace {

// The newest versions that add no events. wl_surface gets new events in
// wl_compositor v6, and xdg_popup gets xdg_popup.repositioned in xdg_wm_base
// v3. A listener written for v1 handles every event up to these versions.
constexpr std::uint32_t kCompositorVersion{5};
constexpr std::uint32_t kWmBaseVersion{2};

} // namespace

auto record_global(FoundGlobals& found, std::uint32_t name,
                   std::string_view interface, std::uint32_t version) -> void {
    GlobalEntry entry{.name = name, .version = version};

    if (interface == "wl_compositor") {
        found.compositor = entry;
    } else if (interface == "xdg_wm_base") {
        found.wm_base = entry;
    }
}

auto choose_globals(FoundGlobals const& found)
    -> std::expected<RequiredGlobals, WindowError> {
    if (!found.compositor) {
        return std::unexpected{WindowError::MissingCompositor};
    }
    if (!found.wm_base) {
        return std::unexpected{WindowError::MissingXdgWmBase};
    }

    // Binding a version newer than the compositor offers is a protocol error.
    return RequiredGlobals{
        .compositor = {.name = found.compositor->name,
                       .version = std::min(found.compositor->version,
                                           kCompositorVersion)},
        .wm_base = {.name = found.wm_base->name,
                    .version =
                        std::min(found.wm_base->version, kWmBaseVersion)},
    };
}

} // namespace engine::platform::wayland
