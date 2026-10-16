add_library(engine_runtime INTERFACE)
add_library(engine::runtime ALIAS engine_runtime)

target_include_directories(
    engine_runtime
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>"
)

target_link_libraries(
    engine_runtime
    INTERFACE engine::project_options engine::project_warnings
)

if(Vulkan_FOUND)
    target_link_libraries(engine_runtime INTERFACE Vulkan::Vulkan)
endif()

if(ENGINE_BUILD_WAYLAND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WaylandClient REQUIRED IMPORTED_TARGET wayland-client)

    include(WaylandProtocols)
    engine_add_xdg_shell_protocol(engine_xdg_shell_protocol)

    add_library(
        engine_platform_wayland
        STATIC
        src/platform/linux/wayland/registry_globals.cpp
        src/platform/linux/wayland/wayland_connection.cpp
    )
    target_link_libraries(
        engine_platform_wayland
        PRIVATE
            engine::project_options
            engine::project_warnings
            PkgConfig::WaylandClient
            engine_xdg_shell_protocol
    )
    target_include_directories(
        engine_platform_wayland
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )
endif()
