// Tests for parse_version and version_component.
// engine_add_gtest() supplies main() and registers each TEST() with CTest.
// See cmake/TestHelpers.cmake and docs/adr/0009-googletest-for-cpu-tests.md.
//
// EXPECT_* continues after a failure. Use ASSERT_* before accessing a value
// that must be valid; these tests compare whole std::expected values instead.
// EXPECT_DEATH runs each assertion check in a separate process.
// Use TEST(), since the MinGW source scan cannot discover TEST_P instances.
#include <gtest/gtest.h>

#include <engine/core/version.hpp>

#include "version_printers.hpp"

using engine::core::parse_version;
using engine::core::Version;
using engine::core::VersionParseError;
using engine::core::version_component;

// Wrap Version{1, 2, 3} in parentheses so its commas stay inside one macro argument.

// parse_version: valid input.

TEST(ParseVersion, ParsesThreeComponents) {
    EXPECT_EQ(parse_version("1.2.3"), (Version{1, 2, 3}));
}

// Catch parsers that assume each component is one digit.
TEST(ParseVersion, ParsesMultiDigitComponents) {
    EXPECT_EQ(parse_version("10.20.30"), (Version{10, 20, 30}));
}

TEST(ParseVersion, ParsesZeroComponents) {
    EXPECT_EQ(parse_version("0.0.0"), (Version{0, 0, 0}));
}

// parse_version: empty input.

// A separate error lets callers distinguish missing input from invalid input.
TEST(ParseVersion, RejectsEmptyInput) {
    EXPECT_EQ(parse_version(""), std::unexpected{VersionParseError::EmptyInput});
}

// parse_version: invalid input.

// Require exactly three components, including a value after the last dot.
TEST(ParseVersion, RejectsWrongComponentCount) {
    EXPECT_EQ(parse_version("1.2"), std::unexpected{VersionParseError::MalformedVersion});
    EXPECT_EQ(parse_version("1.2."), std::unexpected{VersionParseError::MalformedVersion});
    EXPECT_EQ(parse_version("1.2.3.4"), std::unexpected{VersionParseError::MalformedVersion});
}

// Reject letters both within and in place of a component.
TEST(ParseVersion, RejectsNonNumericComponents) {
    EXPECT_EQ(parse_version("1.2.x"), std::unexpected{VersionParseError::MalformedVersion});
    EXPECT_EQ(parse_version("1x2.3"), std::unexpected{VersionParseError::MalformedVersion});
}

// from_chars rejects leading spaces; the final pointer check rejects trailing ones.
TEST(ParseVersion, RejectsSurroundingWhitespace) {
    EXPECT_EQ(parse_version(" 1.2.3"), std::unexpected{VersionParseError::MalformedVersion});
    EXPECT_EQ(parse_version("1.2.3 "), std::unexpected{VersionParseError::MalformedVersion});
}

// Unsigned components make from_chars reject negatives.
// This catches an accidental change to a signed type.
TEST(ParseVersion, RejectsNegativeComponent) {
    EXPECT_EQ(parse_version("1.-2.3"), std::unexpected{VersionParseError::MalformedVersion});
}

// Overflow must return an error, never wrap to a smaller value.
TEST(ParseVersion, RejectsOutOfRangeComponent) {
    EXPECT_EQ(parse_version("4294967296.0.0"),
              std::unexpected{VersionParseError::MalformedVersion});
}

// version_component: valid indices.

TEST(VersionComponent, ReturnsRequestedField) {
    constexpr Version v{1, 2, 3};

    EXPECT_EQ(version_component(v, 0), 1u);
    EXPECT_EQ(version_component(v, 1), 2u);
    EXPECT_EQ(version_component(v, 2), 3u);
}

// version_component: invalid indices.
// GoogleTest runs DeathTest suites first to reduce problems with fork and threads.
// Match "index" in stderr to identify the assertion across toolchains.

TEST(VersionComponentDeathTest, RejectsIndexAboveRange) {
    constexpr Version v{1, 2, 3};

    // Index 3 is a caller bug: Version has only three fields.
    EXPECT_DEATH(static_cast<void>(version_component(v, 3)), "index");
}

// A signed index lets the assertion check negatives before they wrap to unsigned.
TEST(VersionComponentDeathTest, RejectsNegativeIndex) {
    constexpr Version v{1, 2, 3};

    EXPECT_DEATH(static_cast<void>(version_component(v, -1)), "index");
}
