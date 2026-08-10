#pragma once
#include <cassert>
#include <expected>
#include <string_view>
#include <utility>
#include <charconv>
#include <array>

namespace engine::core {

enum class VersionParseError {
    EmptyInput,
    MalformedVersion,
};

struct Version {
    unsigned int major;
    unsigned int minor;
    unsigned int patch;

    // Equality only, on purpose. A defaulted <=> would compare major, then
    // minor, then patch -- correct for semver precedence today only because the
    // field order happens to match. Adding a pre-release field later would make
    // the defaulted ordering silently wrong ("" sorts before "alpha", but semver
    // ranks 1.0.0-alpha *below* 1.0.0), with no compile error to catch it.
    // Nothing needs ordering yet; adding <=> later is not a breaking change.
    friend bool operator==(Version const&, Version const&) = default;
};

// Both need a real definition here, marked `inline` -- engine::runtime is a
// header-only INTERFACE target (see engine/CMakeLists.txt), so a
// non-inline free-function definition pulled in by two translation units
// (apps/sandbox/main.cpp and a future test) would violate the One
// Definition Rule at link time.

[[nodiscard]] inline std::expected<Version, VersionParseError> parse_version(std::string_view text) {
    if (text.empty()) return std::unexpected{VersionParseError::EmptyInput};

    char const* const last = text.data() + text.size();
    char const* cursor = text.data();
    std::array<unsigned int, 3> parts{};

    // Loop invariant: at top of each iteration, cursor points at the first character of the next component.
    for (std::size_t i = 0; i < parts.size(); ++i) {
        const auto [ptr, ec] = std::from_chars(cursor, last, parts[i]);

        if (ec != std::errc{}) {
            return std::unexpected{VersionParseError::MalformedVersion};
        }
        if (i == 2 && ptr != last) {
            return std::unexpected{VersionParseError::MalformedVersion};
        }
        if (i < 2 && (ptr == last || *ptr != '.')) {
            return std::unexpected{VersionParseError::MalformedVersion};
        }

        if (i < 2) {
            cursor = ptr + 1;
        }
    }

    return Version{parts[0], parts[1], parts[2]};
}

[[nodiscard]] inline unsigned int version_component(Version const& v, int index) {
    assert(index >= 0 && index <= 2);
    switch (index) {
        case 0: return v.major;
        case 1: return v.minor;
        case 2: return v.patch;
    }
    std::unreachable();
}

} // namespace engine::core
