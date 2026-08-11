// GoogleTest value printers for engine::core's version types.
//
// Without these, a failed EXPECT_EQ prints a hex dump rather than a value:
//
//   parse_version("1.2")
//     Which is: 16-byte object <01-00 00-00 67-02 00-00 F0-37 4C-7B 00-02 00-00>
//
// GoogleTest finds PrintTo by argument-dependent lookup, so these must live in
// namespace engine::core -- but the *file* belongs here in tests/, not in the
// engine headers. PrintTo is GoogleTest vocabulary, and the engine's public
// interface should not carry a test framework's naming convention.
//
// ADL reaches the std::expected and std::unexpected overloads because template
// arguments contribute to the associated namespace set: instantiating them with
// engine::core types associates engine::core. That is what makes this possible
// without opening namespace std, which for a non-program-defined type would be
// undefined behavior.
//
// All four overloads are load-bearing. Drop the std::unexpected one and an
// EXPECT_EQ against std::unexpected{...} hex-dumps its right-hand operand even
// though the other three are present.
#pragma once
#include <expected>
#include <ostream>

#include <engine/core/version.hpp>

namespace engine::core {

inline auto PrintTo(Version const& v, std::ostream* os) -> void {
    *os << v.major << '.' << v.minor << '.' << v.patch;
}

// No default: label on purpose. With every enumerator listed and no default,
// adding a third VersionParseError produces a -Wswitch warning pointing here
// instead of silently printing a fallback forever. The line after the switch
// handles a value cast in from outside the enumerator set.
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
