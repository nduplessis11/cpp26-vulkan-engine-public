// Death test: proves version_component's invariant is actually enforced.
//
// CTest has no native death-test support, and neither of the obvious properties
// works here. A failed assert raises SIGABRT, which CTest reports as "Subprocess
// aborted" - a result category distinct from a nonzero exit code, and one that both
// WILL_FAIL and PASS_REGULAR_EXPRESSION leave untouched. So the binary converts the
// abort into a normal exit itself, and registers as an ordinary exit-code test.
//
// The shell alternative (`sh -c '! ./binary'`) would also work, but only on the
// POSIX presets - this repo also builds a Windows mingw preset, where SIGABRT still
// exists but `sh` does not.
//
// Keep this binary to the single out-of-contract call: everything after the abort is
// unreachable, so a second assertion in here would be silently untested.
#include <csignal>
#include <cstdlib>

#include <engine/core/version.hpp>

namespace {

// Reached only when the assert fires, which is this test's pass condition. _Exit is
// one of the few async-signal-safe ways out: it skips atexit handlers and stream
// flushing rather than re-entering libc from a signal context.
[[noreturn]] void on_abort(int /*signal*/) {
    std::_Exit(EXIT_SUCCESS);
}

} // namespace

int main() {
    std::signal(SIGABRT, on_abort);

    constexpr engine::core::Version version{1, 2, 3};

    // Out of contract on purpose: Version has exactly 3 fields, so index 3 is a
    // caller bug by construction.
    static_cast<void>(engine::core::version_component(version, 3));

    // Only reachable if the invariant is not being checked - asserts compiled out,
    // or the precondition removed.
    return EXIT_FAILURE;
}
