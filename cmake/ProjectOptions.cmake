include_guard(GLOBAL)

function(engine_define_project_options)
    option(ENGINE_BUILD_TESTS "Build the test targets" ON)
    option(ENGINE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
    option(ENGINE_ENABLE_SANITIZERS "Enable supported sanitizers in debug builds" OFF)
    option(ENGINE_ENABLE_CONTRACTS "Enable GCC contracts with Debug enforcement (OFF is for configure-only Clang tooling)" ON)
    option(ENGINE_ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)
    option(ENGINE_REQUIRE_VULKAN_SDK "Require Vulkan 1.4 during configure" ON)

    set(
        ENGINE_PLATFORM_BACKEND
        "AUTO"
        CACHE STRING
        "Platform backend to build: AUTO, HEADLESS, WAYLAND, XCB, or WIN32"
    )
    set_property(CACHE ENGINE_PLATFORM_BACKEND PROPERTY STRINGS AUTO HEADLESS WAYLAND XCB WIN32)

    if(ENGINE_ENABLE_CLANG_TIDY)
        find_program(ENGINE_CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
        set(CMAKE_CXX_CLANG_TIDY "${ENGINE_CLANG_TIDY_EXECUTABLE}" PARENT_SCOPE)
    endif()
endfunction()
