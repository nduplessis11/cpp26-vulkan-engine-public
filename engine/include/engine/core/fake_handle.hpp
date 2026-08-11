#pragma once
#include <cstdint>
namespace engine::core {

struct FakeDriver {
    std::uint32_t next_id = 1;
    std::uint32_t open_count = 0;
    std::uint32_t close_count = 0;
};

inline std::uint32_t fake_open(FakeDriver& d) {
    ++d.open_count;
    return d.next_id++;
}

inline void fake_close(FakeDriver& d, std::uint32_t id [[maybe_unused]]) {
    ++d.close_count;
}

} // namespace engine::core
