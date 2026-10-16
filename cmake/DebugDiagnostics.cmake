include_guard(GLOBAL)

function(engine_enable_debug_diagnostics target_name)
    # Extend CMake's default -g: -Og helps debugging, -g3 includes macro definitions,
    # and keeping frame pointers helps produce reliable backtraces.
    target_compile_options(
        ${target_name}
        INTERFACE "$<$<CONFIG:Debug>:-Og;-g3;-fno-omit-frame-pointer>"
    )
endfunction()
