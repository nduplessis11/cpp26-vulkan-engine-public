include_guard(GLOBAL)

function(engine_enable_static_windows_runtime target_name)
    if(
        NOT CMAKE_SYSTEM_NAME STREQUAL "Windows"
        OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
    )
        return()
    endif()

    # Link the MinGW runtime into the executable so it needs no separate MinGW DLLs.
    target_link_options(
        ${target_name}
        INTERFACE -static -static-libgcc -static-libstdc++
    )
endfunction()
