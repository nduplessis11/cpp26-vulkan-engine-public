# ADR 0009: GoogleTest for CPU tests, fetched and pinned

- Status: Accepted
- Date: 2026-08-10

## Context

Tests were hand-rolled `int main()` binaries reporting through the process exit
code, one CTest entry per binary. Lesson 002 made the cost visible: four
`parse_version` cases took roughly eight lines each, because the assertion had
to branch on `std::expected`'s discriminant and then compare fields by hand.

The sharper problem was contract tests. A failed `assert` raises SIGABRT, which
CTest classifies as "Subprocess aborted" — a result category neither `WILL_FAIL`
nor `PASS_REGULAR_EXPRESSION` inverts. `tests/unit/version_component_contract_test.cpp`
worked around this by installing a SIGABRT handler and `_Exit(EXIT_SUCCESS)`, and
that workaround imposes one assertion per binary: everything after the abort is
unreachable, so a second death assertion in the same file would be silently
untested. Every future invariant would cost another binary and another three
lines of CMake.

No prior decision recorded a preference against a test framework; the repo simply
had none, and had no mechanism for acquiring third-party source either.

## Decision

Adopt GoogleTest for CPU-side tests (`tests/unit/`, `tests/smoke/`), acquired via
`FetchContent` and pinned to commit `d89aac5f0dd4021198d903d39de16f896726de21`
(the v1.18.x branch head — newer than the v1.17.0 tag, and the revision actually
verified here).

Registration goes through `engine_add_gtest()` in `cmake/TestHelpers.cmake`,
which selects `gtest_discover_tests()` natively and `gtest_add_tests()` when
`CMAKE_CROSSCOMPILING`.

Evidence gathered before deciding, on this project's own toolchain (GCC 17.0.0
trunk, `-std=c++26`, `__cplusplus=202603`):

- GoogleTest builds clean and warning-free for the Linux target and the MinGW
  cross target.
- `GTEST_HAS_DEATH_TEST=1` on both. `EXPECT_DEATH` on a live `assert` passes in
  ~8 ms on Linux and ~60–86 ms for a cross-compiled binary executed on a real
  Windows host. The flag reports style `fast` on both, but the Windows timing is
  consistent with process re-execution rather than `fork()`.
- The existing `-static -static-libgcc -static-libstdc++` link options
  (`cmake/WindowsRuntime.cmake`) keep working, so the test `.exe` still runs on a
  Windows host with no staged MinGW DLLs.
- `gtest_discover_tests()` **fails the cross build** — it executes the freshly
  linked `.exe` on the Linux build host. `gtest_add_tests()` source-scans instead
  and registers all cases correctly.
- Cost: cold build 7.1 s vs 2.0 s; ~0.6 s added per test translation unit; test
  binary 3.59 MB vs 0.72 MB (Linux debug).

## Consequences

Death tests become ordinary assertions, so one binary can hold many invariant
checks and `version_component_contract_test.cpp` is deleted rather than
duplicated. CTest reports per-test rather than per-binary.

Assertion diagnostics are **not** free for engine types. GoogleTest has no
printer for `std::expected` or for `Version` and falls back to a hex dump —
strictly worse than the hand-written messages it replaces. Readable output
requires a `PrintTo` overload per asserted type. These can live in `engine::core`
rather than namespace `std`, because template arguments contribute to the
associated namespace set for ADL.

A configure with no network now fails. Both build images were verified to have
configure-time network; an offline or air-gapped build would need the pin
switched to a vendored copy.

`gtest_add_tests()` only sees test names spelled literally in the source, so
`TEST_P` and `TYPED_TEST` are invisible to the cross build's registration. If
those become necessary, install wine in the windows-mingw image and set
`CMAKE_CROSSCOMPILING_EMULATOR` so `gtest_discover_tests()` works again.

This is the repo's first external source dependency and sets the pattern:
fetched, pinned to an exact commit, justified in an ADR. gmock is off; turn it on
when matchers are actually needed.

GPU and golden-image tests are out of scope. Their hard parts — distinguishing an
unsupported environment from a failure, and image comparison — are unaffected by
this choice, though GoogleTest's subprocess mechanism is a plausible fit for
capability probes that must not take down the run.

[ADR 0010](0010-prefer-language-contracts.md) replaces the engine's runtime
`assert` calls with language contracts. The measurements above record the
original assertion implementation. GoogleTest death tests also verify the
replacement preconditions.
