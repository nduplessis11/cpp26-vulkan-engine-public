#pragma once
#include <cstdint>
namespace engine::core {

struct FakeDriver {
    std::uint32_t next_id = 1;
    std::uint32_t open_count = 0;
    std::uint32_t close_count = 0;
};

[[nodiscard]]
inline auto fake_open(FakeDriver& d) -> std::uint32_t {
    ++d.open_count;
    return d.next_id++;
}

inline auto fake_close(FakeDriver& d, [[maybe_unused]] std::uint32_t id) -> void {
    ++d.close_count;
}

} // namespace engine::core
