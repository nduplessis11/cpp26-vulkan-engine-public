include_guard(GLOBAL)

function(engine_enable_std_print target_name)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows" OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        return()
    endif()

    # These MinGW toolchains need stdc++exp for std::print/println's console helpers.
    # Without it, linking fails on std::__open_terminal. Verified on GCC 16.1 and 17 trunk.
    target_link_libraries(${target_name} INTERFACE stdc++exp)
endfunction()
