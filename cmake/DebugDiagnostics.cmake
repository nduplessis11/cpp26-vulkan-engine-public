include_guard(GLOBAL)

function(engine_enable_debug_diagnostics target_name)
    if(MSVC)
        message(WARNING "The scaffold debug-diagnostics helper does not configure MSVC debug flags yet.")
        return()
    endif()

    # CMake's own Debug config only guarantees "-g" for GCC; it does not pick an
    # optimization level or a frame-pointer policy. -Og keeps stepping and local
    # inspection sane (unlike -O0, it still runs the passes GDB's location lists
    # depend on being self-consistent). -g3 adds macro definitions to DWARF.
    # -fno-omit-frame-pointer keeps backtraces reliable without relying on
    # CFI-only unwinding.
    target_compile_options(${target_name} INTERFACE "$<$<CONFIG:Debug>:-Og;-g3;-fno-omit-frame-pointer>")
endfunction()
