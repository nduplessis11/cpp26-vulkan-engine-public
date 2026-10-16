include_guard(GLOBAL)

function(engine_validate_compiler)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(
            FATAL_ERROR
            "This project pins GCC as its compiler family "
            "(see docs/adr/0001-cpp26-mode-progressive-adoption.md). "
            "CMAKE_CXX_COMPILER_ID reported '${CMAKE_CXX_COMPILER_ID}' "
            "for CMAKE_CXX_COMPILER='${CMAKE_CXX_COMPILER}'. "
            "Select the pinned GCC toolchain explicitly: on Linux use the "
            "linux-gcc-debug preset (or -DCMAKE_CXX_COMPILER=g++); on Windows open an "
            "MSYS2 MinGW shell and use the windows-gcc-debug preset. MSVC is not supported."
        )
    endif()

    if(
        CMAKE_CROSSCOMPILING
        AND CMAKE_SYSTEM_NAME STREQUAL "Windows"
        AND NOT CMAKE_CXX_COMPILER MATCHES "mingw32"
    )
        message(
            FATAL_ERROR
            "Windows cross-compilation requires the x86_64-w64-mingw32 GCC toolchain "
            "(see cmake/toolchains/mingw-x86_64.cmake), but CMAKE_CXX_COMPILER resolved to "
            "'${CMAKE_CXX_COMPILER}'. Without this check a misconfigured preset would silently "
            "link against the host compiler instead of failing configure."
        )
    endif()

    message(
        STATUS
        "Compiler identity check passed: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} (${CMAKE_CXX_COMPILER})"
    )
endfunction()
