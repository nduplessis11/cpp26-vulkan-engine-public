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

    # Enable contracts in Debug, like assert(). The preview's default "enforce"
    # mode terminates the program when a contract fails.
    target_compile_options(${target_name} INTERFACE "$<$<CONFIG:Debug>:-fcontracts>")
    message(STATUS "GCC contracts preview enabled for Debug builds (-fcontracts): pre()/post()/contract_assert() are live.")
endfunction()
