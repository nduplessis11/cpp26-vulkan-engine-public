include_guard(GLOBAL)

function(engine_enable_std_print target_name)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows" OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        return()
    endif()

    # GCC ships std::print's Windows console-writing helpers (__open_terminal,
    # __write_to_terminal) in the separate "experimental" archive until they graduate
    # into libstdc++.a proper -- the same mechanism historically used for
    # std::stacktrace. Without this, std::print()/std::println() to stdout/stderr
    # fail at link time with "undefined reference to std::__open_terminal". Confirmed
    # missing on both a local prebuilt GCC 16.1 mingw install and this project's own
    # from-source GCC 17 trunk cross-toolchain; -lstdc++exp fixes both.
    target_link_libraries(${target_name} INTERFACE stdc++exp)
endfunction()
