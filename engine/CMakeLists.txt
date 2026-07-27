add_library(engine_runtime INTERFACE)
add_library(engine::runtime ALIAS engine_runtime)

target_include_directories(
    engine_runtime
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>"
)

target_link_libraries(engine_runtime INTERFACE engine::project_options engine::project_warnings)

if(Vulkan_FOUND)
    target_link_libraries(engine_runtime INTERFACE Vulkan::Vulkan)
endif()
