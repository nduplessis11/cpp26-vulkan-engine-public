#pragma once

#include <cstdint>
#include <engine/platform/window.hpp>
#include <expected>
#include <optional>
#include <string_view>

namespace engine::platform::wayland {

struct GlobalEntry {
    std::uint32_t name;
    std::uint32_t version;
    constexpr auto operator==(GlobalEntry const&) const -> bool = default;
};

struct FoundGlobals {
    std::optional<GlobalEntry> compositor;
    std::optional<GlobalEntry> wm_base;
    constexpr auto operator==(FoundGlobals const&) const -> bool = default;
};

struct RequiredGlobals {
    GlobalEntry compositor;
    GlobalEntry wm_base;
    constexpr auto operator==(RequiredGlobals const&) const -> bool = default;
};

auto record_global(FoundGlobals& found, std::uint32_t name,
                   std::string_view interface, std::uint32_t version) -> void;

[[nodiscard]]
auto choose_globals(FoundGlobals const& found)
    -> std::expected<RequiredGlobals, WindowError>;

} // namespace engine::platform::wayland
