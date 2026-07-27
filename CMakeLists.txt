cmake_minimum_required(VERSION 3.25)

project(
    Cpp26VulkanEngine
    VERSION 0.0.0
    DESCRIPTION "Teaching-first C++26 Vulkan 1.4 engine scaffold"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(ProjectOptions)
include(CompilerWarnings)
include(Sanitizers)

engine_define_project_options()

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(engine_project_options INTERFACE)
add_library(engine::project_options ALIAS engine_project_options)
target_compile_features(engine_project_options INTERFACE cxx_std_26)

add_library(engine_project_warnings INTERFACE)
add_library(engine::project_warnings ALIAS engine_project_warnings)
engine_set_project_warnings(engine_project_warnings)
engine_enable_sanitizers(engine_project_options)

find_package(Vulkan 1.4 QUIET)

if(Vulkan_FOUND)
    message(STATUS "Found Vulkan ${Vulkan_VERSION}")
    target_compile_definitions(engine_project_options INTERFACE ENGINE_HAS_VULKAN_SDK=1)
else()
    if(ENGINE_REQUIRE_VULKAN_SDK)
        message(FATAL_ERROR
            "A Vulkan 1.4 SDK/loader/header set was not found. "
            "Install a Vulkan 1.4 SDK or configure only for scaffold inspection with "
            "-DENGINE_REQUIRE_VULKAN_SDK=OFF."
        )
    endif()
    message(WARNING "Building scaffold without Vulkan. Lesson 000 is not complete until Vulkan 1.4 is found.")
    target_compile_definitions(engine_project_options INTERFACE ENGINE_HAS_VULKAN_SDK=0)
endif()

add_subdirectory(engine)
add_subdirectory(apps/sandbox)

if(ENGINE_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()
