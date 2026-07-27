#include <cstdlib>
#include <iostream>

#include <engine/core/scaffold_info.hpp>

#if ENGINE_HAS_VULKAN_SDK
#include <vulkan/vulkan_core.h>
#endif

int main() {
    std::cout << engine::core::scaffold_name() << '\n';
    std::cout << "Scaffold version: " << engine::core::scaffold_version() << '\n';
    std::cout << "__cplusplus: " << __cplusplus << '\n';

#if ENGINE_HAS_VULKAN_SDK
    std::cout << "Vulkan header: "
              << VK_API_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE) << '.'
              << VK_API_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE) << '.'
              << VK_API_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE) << '\n';
#else
    std::cout << "Vulkan 1.4 SDK: not found (scaffold-only configuration)\n";
#endif

    std::cout << "Current lesson: 000 - bootstrap the build. No engine is implemented yet.\n";
    return EXIT_SUCCESS;
}
