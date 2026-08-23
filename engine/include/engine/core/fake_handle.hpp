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

class FakeHandle {
  public:
    FakeHandle() = default;
    explicit FakeHandle(FakeDriver& driver) : driver_{&driver}, id_{fake_open(driver)} {}

    ~FakeHandle() {
        if (is_valid()) {
            fake_close(*driver_, id_);
        }
    }

    [[nodiscard]]
    auto is_valid() const -> bool {
        return id_ != 0;
    }

  private:
    FakeDriver* driver_{};
    std::uint32_t id_{};
};

} // namespace engine::core
