#include <engine/core/scaffold_info.hpp>

auto main() -> int {
    return engine::core::scaffold_name().empty() ? 1 : 0;
}
