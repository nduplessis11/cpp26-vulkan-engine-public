#include <engine/core/scaffold_info.hpp>

int main() {
    return engine::core::scaffold_name().empty() ? 1 : 0;
}
