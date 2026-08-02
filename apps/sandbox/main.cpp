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

    std::cout << "Current lesson: 001 - make warnings and diagnostics intentional.\n";

    // TODO(human): write 2-3 lines that deliberately trip ONE warning from the
    // project policy in cmake/CompilerWarnings.cmake. Predict the flag and the
    // message before you build. This code is temporary and comes back out.

    return EXIT_SUCCESS;
}
