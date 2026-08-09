// Behavior test for engine::core::parse_version / version_component.
//
// No test framework is wired in this repo (see tests/CMakeLists.txt) - CTest reads
// the process exit code and nothing else: 0 passes, anything else fails.
//
// Reporting policy here: count failures, print each one, exit nonzero at the end.
// That trades a precise single failure site for seeing every broken case in one run,
// which matters because MalformedVersion has ~8 interesting inputs. To flip to
// first-failure-exits instead, replace `++failures` with `return 1`.
//
// The design note's proof is all three parse_version outcomes (success, EmptyInput,
// MalformedVersion) plus one in-contract version_component call. The out-of-contract
// call lives in version_component_contract_test.cpp, because a test that aborts
// cannot also report on anything after it.
#include <print>

#include <engine/core/version.hpp>

int main() {
    using engine::core::parse_version;
    using engine::core::Version;
    using engine::core::VersionParseError;
    using engine::core::version_component;

    int failures = 0;

    // Success case. Note the branch on `parsed` comes first: parsed->major is only
    // valid on the success side of std::expected.
    const auto parsed = parse_version("1.2.3");
    if (!parsed) {
        std::println("FAIL  parse_version(\"1.2.3\") returned an error, wanted 1.2.3");
        ++failures;
    } else if (parsed->major != 1 || parsed->minor != 2 || parsed->patch != 3) {
        std::println("FAIL  parse_version(\"1.2.3\") -> {}.{}.{}, wanted 1.2.3", parsed->major,
                     parsed->minor, parsed->patch);
        ++failures;
    }

    // TODO(human)
    const auto parsed_empty = parse_version("");
    if (parsed_empty) {
        std::println("FAIL  parse_version(\"\") -> #.#.#, wanted EmptyInput");
        ++failures;
    } else if (parsed_empty.error() != VersionParseError::EmptyInput) {
        std::println("FAIL  parse_version(\"\") -> !EmptyInput, wanted EmptyInput");
        ++failures;
    }

    const auto parsed_two_digits = parse_version("1.2");
    if (parsed_two_digits) {
        std::println("FAIL  parse_version(\"1.2\") -> #.#, wanted MalformedVersion");
        ++failures;
    } else if (parsed_two_digits.error() != VersionParseError::MalformedVersion) {
        std::println("FAIL  parse_version(\"1.2\") -> !MalformedVersion, wanted MalformedVersion");
        ++failures;
    }

    if (failures > 0) {
        std::println("{} check(s) failed", failures);
        return 1;
    }
    return 0;
}
