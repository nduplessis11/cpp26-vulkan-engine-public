include_guard(GLOBAL)
include(CheckCXXSourceCompiles)

function(engine_enable_contracts target_name)
    if(NOT ENGINE_ENABLE_CONTRACTS)
        # The configure-only Clang tooling preset supplies macros when analyzing code.
        message(
            STATUS
            "Contracts disabled for Clang tooling; this configuration is not for engine builds."
        )
        return()
    endif()

    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(
            FATAL_ERROR
            "Engine builds require GCC contract support; see docs/adr/0010-prefer-language-contracts.md."
        )
    endif()

    set(CMAKE_REQUIRED_FLAGS
        "${CMAKE_REQUIRED_FLAGS} -fcontracts -fcontract-evaluation-semantic=enforce"
    )
    set(contract_probe
        [=[
        #if !defined(__cpp_contracts) || __cpp_contracts < 202502L
        #error GCC contract support is required
        #endif
        auto positive(int value) -> int pre(value > 0) post(result: result > 0) {
            contract_assert(value > 0);
            return value;
        }
        auto main() -> int { return positive(1) - 1; }
    ]=]
    )
    check_cxx_source_compiles(
        "${contract_probe}"
        ENGINE_COMPILER_SUPPORTS_CONTRACTS
    )

    if(NOT ENGINE_COMPILER_SUPPORTS_CONTRACTS AND MINGW)
        # The pinned MinGW library's weak handler can remain unresolved even
        # with -fcontracts. Probe a strong definition that calls the default handler.
        set(handler_source
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/compat/mingw_contract_handler.cpp"
        )
        file(READ "${handler_source}" handler_code)
        check_cxx_source_compiles(
            "${handler_code}\n${contract_probe}"
            ENGINE_COMPILER_SUPPORTS_CONTRACTS_WITH_HANDLER
        )
        if(ENGINE_COMPILER_SUPPORTS_CONTRACTS_WITH_HANDLER)
            target_sources(${target_name} INTERFACE "${handler_source}")
            message(
                STATUS
                "MinGW contracts use a forwarding definition for the default handler."
            )
        endif()
    endif()

    if(NOT ENGINE_COMPILER_SUPPORTS_CONTRACTS)
        if(NOT MINGW OR NOT ENGINE_COMPILER_SUPPORTS_CONTRACTS_WITH_HANDLER)
            message(
                FATAL_ERROR
                "GCC must compile and link pre/post/contract_assert with -fcontracts. "
                "Use a supported GCC toolchain; see docs/adr/0010-prefer-language-contracts.md."
            )
        endif()
    endif()

    # Every build parses contracts. Only Debug evaluates their predicates.
    target_compile_options(
        ${target_name}
        INTERFACE
            -fcontracts
            "$<IF:$<CONFIG:Debug>,-fcontract-evaluation-semantic=enforce,-fcontract-evaluation-semantic=ignore>"
    )

    # GCC 16 and 17 give std::source_location internal linkage when a contract clause
    # is parsed before <source_location>. A class that later stores one, such as
    # LogRecord in logger.hpp, then warns under -Wsubobject-linkage. Including the
    # header before every file avoids it. Remove this when GCC bug 127251 is fixed.
    # SHELL: stops CMake's option de-duplication from splitting the two words.
    target_compile_options(
        ${target_name}
        INTERFACE "SHELL:-include source_location"
    )

    # GCC selects the contracts support library through this link flag.
    target_link_options(${target_name} INTERFACE -fcontracts)
    message(
        STATUS
        "GCC contracts enabled: enforce in Debug, ignore in other configurations."
    )
endfunction()
