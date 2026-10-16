#pragma once
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>
#include <system_error>
#include <utility>

namespace engine::core {

enum class VersionParseError : std::uint8_t {
    EmptyInput,
    MalformedVersion,
};

struct Version {
    unsigned int major;
    unsigned int minor;
    unsigned int patch;

    // Only equality is needed. Default <=> ordering would be wrong if we later
    // add pre-release labels: 1.0.0-alpha must sort before 1.0.0.
    friend auto operator==(Version const&, Version const&) -> bool = default;
};

// Both functions are defined in this header. inline lets multiple source
// files include them without duplicate-definition errors at link time.

[[nodiscard]]
inline auto parse_version(std::string_view text)
    -> std::expected<Version, VersionParseError> {
    if (text.empty())
        return std::unexpected{VersionParseError::EmptyInput};

    char const* const last = text.data() + text.size();
    char const* cursor = text.data();
    std::array<unsigned int, 3> parts{};

    // Each iteration starts at the next component, or at the end if it is
    // missing.
    for (std::size_t i = 0; i < parts.size(); ++i) {
        auto const [ptr, ec] = std::from_chars(cursor, last, parts[i]);

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

[[nodiscard]]
inline auto version_component(Version const& v, int index)
    -> unsigned int pre(index >= 0 && index <= 2) {
    switch (index) {
    case 0:
        return v.major;
    case 1:
        return v.minor;
    case 2:
        return v.patch;
    // The precondition checks the range in Debug; unreachable tells the
    // compiler that callers must never pass another index.
    default:
        std::unreachable();
    }
}

} // namespace engine::core
