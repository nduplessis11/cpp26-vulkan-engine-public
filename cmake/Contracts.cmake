include_guard(GLOBAL)
include(CheckCXXCompilerFlag)

function(engine_enable_contracts target_name)
    if(NOT ENGINE_ENABLE_CONTRACTS)
        return()
    endif()

    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(WARNING "ENGINE_ENABLE_CONTRACTS requested but the compiler isn't GCC; skipping -fcontracts.")
        return()
    endif()

    check_cxx_compiler_flag(-fcontracts ENGINE_COMPILER_SUPPORTS_FCONTRACTS)
    if(NOT ENGINE_COMPILER_SUPPORTS_FCONTRACTS)
        message(WARNING
            "ENGINE_ENABLE_CONTRACTS requested but this GCC build doesn't accept -fcontracts "
            "(needs the GCC 16.1+ contracts preview, see docs/BUILDING.md and "
            "curriculum/lectures/002_choose-error-and-contract-conventions.md). Skipping."
        )
        return()
    endif()

    # Debug-only: this is a checked-invariant facility (see DebugDiagnostics.cmake for
    # the same CONFIG:Debug convention), and the preview's default "enforce" semantic
    # terminates on violation, so it belongs in the same debug-time checking category
    # as assert().
    target_compile_options(${target_name} INTERFACE "$<$<CONFIG:Debug>:-fcontracts>")
    message(STATUS "GCC contracts preview enabled for Debug builds (-fcontracts): pre()/post()/contract_assert() are live.")
endfunction()
