include_guard(GLOBAL)
include(GoogleTest)

# Register each GoogleTest case with CTest.
# Native builds run the binary to list tests with gtest_discover_tests().
# Cross builds use gtest_add_tests() to scan source, since Linux cannot run the
# Windows binary directly. The scan misses TEST_P, TYPED_TEST, and macro-generated
# cases. Supporting those would require Wine and CMAKE_CROSSCOMPILING_EMULATOR.
# The MinGW preset only builds; use windows-gcc-debug on Windows to run tests.
function(engine_add_gtest target)
    add_executable(${target} ${ARGN})

    # engine::runtime supplies project flags; gtest_main supplies main().
    # GoogleTest's own sources do not inherit our warning flags.
    target_link_libraries(${target} PRIVATE engine::runtime GTest::gtest_main)

    if(CMAKE_CROSSCOMPILING)
        gtest_add_tests(TARGET ${target} TEST_LIST ${target}_cases)
    else()
        gtest_discover_tests(${target})
    endif()
endfunction()
