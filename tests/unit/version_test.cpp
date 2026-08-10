// Behavior tests for engine::core::parse_version / version_component.
//
// Registered via engine_add_gtest() (cmake/TestHelpers.cmake), which links
// GTest::gtest_main -- hence no main() here -- and gives CTest one entry per
// TEST() case. See docs/adr/0009-googletest-for-cpu-tests.md for why GoogleTest,
// and cmake/TestHelpers.cmake for why registration differs under the MinGW
// cross build.
//
// Reporting policy: EXPECT_* throughout, so one broken case does not hide the
// rest. ASSERT_* stops at the first failure within its own TEST, and is
// mandatory rather than stylistic wherever a later line dereferences something
// an earlier line validated. Comparing whole std::expected values sidesteps
// that entirely, which is why nothing here dereferences.
//
// This file now also carries the out-of-contract death tests. They used to need
// their own binary: a failed assert raises SIGABRT, and the workaround that
// converted it into a clean exit left everything after the abort unreachable,
// so one binary could hold exactly one death assertion. EXPECT_DEATH runs each
// in a subprocess, so that ceiling is gone.
//
// Deliberately no TEST_P: the MinGW cross build registers tests by scanning
// this source for TEST() macros, and parameterized instantiations are invisible
// to that scan. Grouped EXPECT_EQs cover the same ground portably.
#include <gtest/gtest.h>

#include <engine/core/version.hpp>

#include "version_printers.hpp"

using engine::core::parse_version;
using engine::core::Version;
using engine::core::VersionParseError;
using engine::core::version_component;

// EXPECT_EQ is a macro, so a braced initializer's commas split it into extra
// arguments. Version{1, 2, 3} must be parenthesized; std::unexpected{...}
// contains no comma and does not.

// --- parse_version: success ------------------------------------------------

TEST(ParseVersion, ParsesThreeComponents) {
    EXPECT_EQ(parse_version("1.2.3"), (Version{1, 2, 3}));
}

// Multi-digit components: a hardcoded return, or a cursor that advances one
// character per component, passes the single-digit case and fails here.
TEST(ParseVersion, ParsesMultiDigitComponents) {
    EXPECT_EQ(parse_version("10.20.30"), (Version{10, 20, 30}));
}

TEST(ParseVersion, ParsesZeroComponents) {
    EXPECT_EQ(parse_version("0.0.0"), (Version{0, 0, 0}));
}

// --- parse_version: EmptyInput ---------------------------------------------

// The one input that is not merely malformed. Empty gets its own error so a
// caller can tell "nothing was supplied" from "what was supplied is wrong" --
// an unset config key versus a typo'd one.
TEST(ParseVersion, RejectsEmptyInput) {
    EXPECT_EQ(parse_version(""), std::unexpected{VersionParseError::EmptyInput});
}

// --- parse_version: MalformedVersion, one case per rejection branch ---------

// Fewer or more than three components. "1.2." exercises the same branch as
// "1.2" but with the separator present and the component missing.
TEST(ParseVersion, RejectsWrongComponentCount) {
    EXPECT_EQ(parse_version("1.2"), std::unexpected{VersionParseError::MalformedVersion});
    EXPECT_EQ(parse_version("1.2."), std::unexpected{VersionParseError::MalformedVersion});
    EXPECT_EQ(parse_version("1.2.3.4"), std::unexpected{VersionParseError::MalformedVersion});
}

// Non-numeric text, in the final component and in the middle of the first.
TEST(ParseVersion, RejectsNonNumericComponents) {
    EXPECT_EQ(parse_version("1.2.x"), std::unexpected{VersionParseError::MalformedVersion});
    EXPECT_EQ(parse_version("1x2.3"), std::unexpected{VersionParseError::MalformedVersion});
}

// std::from_chars does not skip leading whitespace, so the leading case fails
// in from_chars itself; the trailing case is caught by the final ptr != last
// check instead. Same error, two different branches.
TEST(ParseVersion, RejectsSurroundingWhitespace) {
    EXPECT_EQ(parse_version(" 1.2.3"), std::unexpected{VersionParseError::MalformedVersion});
    EXPECT_EQ(parse_version("1.2.3 "), std::unexpected{VersionParseError::MalformedVersion});
}

// Pins a rule that lives in a type rather than in a statement. Nothing in
// parse_version says "non-negative"; the components are unsigned, so from_chars
// refuses the '-' itself and reports an errc the existing check already maps to
// MalformedVersion. Switching that array back to a signed type would reopen the
// hole silently -- this case is what would notice.
TEST(ParseVersion, RejectsNegativeComponent) {
    EXPECT_EQ(parse_version("1.-2.3"), std::unexpected{VersionParseError::MalformedVersion});
}

// Same mechanism, other end of the range: one past UINT_MAX yields
// result_out_of_range rather than a wrapped value.
TEST(ParseVersion, RejectsOutOfRangeComponent) {
    EXPECT_EQ(parse_version("4294967296.0.0"),
              std::unexpected{VersionParseError::MalformedVersion});
}

// --- version_component: in contract ----------------------------------------

TEST(VersionComponent, ReturnsRequestedField) {
    constexpr Version v{1, 2, 3};

    EXPECT_EQ(version_component(v, 0), 1u);
    EXPECT_EQ(version_component(v, 1), 2u);
    EXPECT_EQ(version_component(v, 2), 3u);
}

// --- version_component: out of contract ------------------------------------
//
// Suite names ending in "DeathTest" are a GoogleTest convention: it orders those
// suites before the others, because forking a process that has already started
// threads is the classic source of flaky death tests.
//
// The second argument is a regex matched against the dying process's stderr.
// It matches "index" rather than the whole predicate because the toolchains
// format assertion failures differently -- glibc prints
//   Assertion `index >= 0 && index <= 2' failed.
// and the UCRT prints
//   Assertion failed: index >= 0 && index <= 2, file ..., line ...
// Both contain the identifier, so this still proves which assert fired rather
// than merely that the process died.

TEST(VersionComponentDeathTest, RejectsIndexAboveRange) {
    constexpr Version v{1, 2, 3};

    // Version has exactly three fields, so index 3 is a caller bug by
    // construction -- never a recoverable runtime condition.
    EXPECT_DEATH(static_cast<void>(version_component(v, 3)), "index");
}

// index stays a signed int precisely so this input is representable and can be
// caught. An unsigned parameter would make the index >= 0 half of the
// precondition vacuously true and silently wrap a caller's -1 into a huge value.
TEST(VersionComponentDeathTest, RejectsNegativeIndex) {
    constexpr Version v{1, 2, 3};

    EXPECT_DEATH(static_cast<void>(version_component(v, -1)), "index");
}
