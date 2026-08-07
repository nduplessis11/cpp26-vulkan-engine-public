#pragma once
#include <cassert>
#include <expected>
#include <string_view>
#include <utility>
#include <charconv>

namespace engine::core {

enum class VersionParseError {
    EmptyInput,
    MalformedVersion,
};

struct Version {
    int major;
    int minor;
    int patch;
};

// TODO(human): implement parse_version and version_component below, per
// curriculum/design-notes/002_choose-error-and-contract-conventions.md.
//
// parse_version(""): VersionParseError::EmptyInput. Anything that isn't
// exactly three dot-separated non-negative integers, no extra characters or
// whitespace ("1.2", "1.2.3.4", "1.2.x", " 1.2.3"): MalformedVersion.
// "1.2.3": Version{1, 2, 3}.
//
// version_component: assert(index >= 0 && index <= 2) -- an out-of-range
// index is always a caller bug (this type only has 3 fields), never a
// real-world condition to hand back to a caller. Return the matching
// major/minor/patch field for index 0/1/2.
//
// Both need a real definition here, marked `inline` -- engine::runtime is a
// header-only INTERFACE target (see engine/CMakeLists.txt), so a
// non-inline free-function definition pulled in by two translation units
// (apps/sandbox/main.cpp and a future test) would violate the One
// Definition Rule at link time.

[[nodiscard]] inline std::expected<Version, VersionParseError> parse_version(std::string_view text) {
    if (text.empty()) return std::unexpected{VersionParseError::EmptyInput};

    int value = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
    std::println("\"{}\" -> value={} stopped_at={} of {} ok={}",
                 text, value, ptr - text.data(), text.size(), ec == std::errc{});

    return Version{1, 2, 3};
}

[[nodiscard]] inline int version_component(const Version& v, int index) {
    assert(index >= 0 && index <= 2);
    switch (index) {
        case 0: return v.major;
        case 1: return v.minor;
        case 2: return v.patch;
    }
    std::unreachable();
}

} // namespace engine::core
