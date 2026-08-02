include_guard(GLOBAL)

function(engine_enable_static_windows_runtime target_name)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows" OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        return()
    endif()

    # Statically linking the MinGW runtime means the produced .exe depends only on
    # core Windows DLLs -- nothing to stage next to it and nothing to accidentally
    # pick up from whichever MSYS2 install happens to be on PATH.
    target_link_options(${target_name} INTERFACE -static -static-libgcc -static-libstdc++)
endfunction()
