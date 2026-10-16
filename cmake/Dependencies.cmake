include_guard(GLOBAL)
include(FetchContent)

# Fetch GoogleTest at configure time, pinned to a tested commit. A fresh fetch
# needs network access. See docs/adr/0009-googletest-for-cpu-tests.md.
# This revision built cleanly with GCC 17 trunk in C++26 mode on Linux and MinGW.
# Repeat those checks before changing the pin.
# Omit GIT_SHALLOW because GIT_TAG uses a commit hash.
function(engine_fetch_test_dependencies)
    # GoogleTest is only needed to build and run tests.
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    # Current assertions and death tests only need gtest.
    # Enable gmock when tests need its mocks or matchers, such as EXPECT_THAT.
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG
            d89aac5f0dd4021198d903d39de16f896726de21 # Tested v1.18.x commit.
    )
    # Skip clang-tidy for GoogleTest's sources. HeaderFilterRegex only filters
    # headers; it cannot exclude the main source file being analyzed.
    set(ENGINE_SAVED_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY}")
    unset(CMAKE_CXX_CLANG_TIDY)

    FetchContent_MakeAvailable(googletest)

    set(CMAKE_CXX_CLANG_TIDY "${ENGINE_SAVED_CLANG_TIDY}")
endfunction()
