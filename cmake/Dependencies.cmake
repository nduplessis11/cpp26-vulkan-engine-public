include_guard(GLOBAL)
include(FetchContent)

# GoogleTest is the repo's first external source dependency, so this file also
# establishes how third-party source enters the project: fetched at configure
# time, pinned to an exact commit, never vendored into the tree.
#
# Fetching rather than submoduling: both build images have network at configure
# time (verified with `git ls-remote` from inside the linux image), and a pinned
# commit is exactly as reproducible as a submodule without adding googletest's
# file count to this repo or a `--recurse-submodules` trap for a fresh clone.
# The cost is real and worth stating: a configure with no network now fails.
#
# Pinned to a commit rather than a tag on purpose. The newest tagged release is
# v1.17.0; this commit is the v1.18.x branch head, and it is the revision
# actually verified against this project's toolchain -- GCC 17.0.0 trunk,
# -std=c++26 (__cplusplus=202603), building clean and warning-free for both the
# native Linux target and the MinGW cross target. Re-run that check before
# moving the pin.
#
# GIT_SHALLOW is deliberately absent: CMake documents it as unreliable when
# GIT_TAG names a commit hash instead of a branch or tag.
function(engine_fetch_test_dependencies)
    # Test-only dependency: nothing here belongs in an install tree.
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    # gmock stays off until something needs it. It roughly doubles the
    # dependency's build time, and this engine's explicit-ownership architecture
    # (see ADR 0007) produces few of the interface seams gmock exists to fake.
    # Flip to ON when you want gmock's matchers (EXPECT_THAT/ElementsAre); the
    # death-test and assertion macros used today live in gtest proper.
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG d89aac5f0dd4021198d903d39de16f896726de21 # v1.18.x branch head
    )
    FetchContent_MakeAvailable(googletest)
endfunction()
