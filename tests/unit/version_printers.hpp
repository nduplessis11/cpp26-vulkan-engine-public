// Print readable version values in GoogleTest failures.
// Keep these helpers in tests/ so engine headers stay independent of GoogleTest.
//
// Argument-dependent lookup (ADL) finds PrintTo in engine::core, including for
// expected/unexpected with engine::core template arguments. No additions to std
// are needed. Keep all four overloads so neither side of EXPECT_EQ prints raw bytes.
#pragma once
#include <expected>
#include <ostream>

#include <engine/core/version.hpp>

namespace engine::core {

inline auto PrintTo(Version const& v, std::ostream* os) -> void {
    *os << v.major << '.' << v.minor << '.' << v.patch;
}

// Omit default so -Wswitch warns when a new error is missing here.
// The fallback prints values outside the enum's named cases.
inline auto PrintTo(VersionParseError e, std::ostream* os) -> void {
    switch (e) {
        case VersionParseError::EmptyInput:       *os << "EmptyInput"; return;
        case VersionParseError::MalformedVersion: *os << "MalformedVersion"; return;
    }
    *os << "VersionParseError(" << static_cast<int>(e) << ')';
}

inline auto PrintTo(std::unexpected<VersionParseError> const& u, std::ostream* os) -> void {
    *os << "unexpected ";
    PrintTo(u.error(), os);
}

inline auto PrintTo(std::expected<Version, VersionParseError> const& r, std::ostream* os) -> void {
    if (r) {
        PrintTo(*r, os);
        return;
    }
    *os << "unexpected ";
    PrintTo(r.error(), os);
}

} // namespace engine::core
