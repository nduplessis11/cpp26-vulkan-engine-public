#include <cstdio>
#include <cstdlib>
#include <engine/core/logger.hpp>
#include <engine/core/scaffold_info.hpp>
#include <exception>
#include <format>

#if ENGINE_HAS_VULKAN_SDK
#include <vulkan/vulkan_core.h>
#endif

namespace {

using engine::core::Category;
using engine::core::Logger;
using engine::core::Severity;
using engine::core::StderrSink;

constexpr Category kSandbox{"sandbox"};
constexpr Category kVulkan{"vulkan"};

auto run() -> int {
    // Declare the sink first so it outlives the logger.
    StderrSink sink;
    Logger logger{sink, Severity::Debug};

    logger.log(Severity::Info, kSandbox, engine::core::scaffold_name());
    logger.log(Severity::Info, kSandbox,
               std::format("scaffold version {}", engine::core::scaffold_version()));
    logger.log(Severity::Debug, kSandbox, std::format("__cplusplus {}", __cplusplus));

#if ENGINE_HAS_VULKAN_SDK
    logger.log(Severity::Info, kVulkan,
               std::format("header {}.{}.{}", VK_API_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE),
                           VK_API_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE),
                           VK_API_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE)));
#else
    logger.log(Severity::Warn, kVulkan, "1.4 SDK not found; scaffold-only configuration");
#endif

    return EXIT_SUCCESS;
}

} // namespace

auto main() -> int {
    try {
        return run();
    } catch (std::exception const& error) {
        // Report directly; formatting or allocating a message could fail again.
        std::fputs("[error] sandbox: ", stderr);
        std::fputs(error.what(), stderr);
        std::fputs("\n", stderr);
        return EXIT_FAILURE;
    }
}
