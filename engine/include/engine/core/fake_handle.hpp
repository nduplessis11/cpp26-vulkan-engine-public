#pragma once
#include <cassert>
#include <cstdint>

namespace engine::core {

struct FakeDriver {
    std::uint32_t next_id = 1;
    std::uint32_t open_count = 0;
    std::uint32_t close_count = 0;
};

[[nodiscard]]
inline auto fake_open(FakeDriver& d) noexcept -> std::uint32_t {
    ++d.open_count;
    return d.next_id++;
}

inline auto fake_close(FakeDriver& d, [[maybe_unused]] std::uint32_t id) noexcept -> void {
    ++d.close_count;
}

class FakeHandle {
  public:
    FakeHandle() = default;
    explicit FakeHandle(FakeDriver& driver) : driver_{&driver}, id_{fake_open(driver)} {}

    ~FakeHandle() {
        reset();
    }

    FakeHandle(const FakeHandle&) = delete;
    auto operator=(const FakeHandle&) -> FakeHandle& = delete;

    // A moved-from handle is left in the default-constructed state, so
    // releasing it is a no-op and the stolen id is closed exactly once.
    FakeHandle(FakeHandle&& other) noexcept : driver_{other.driver_}, id_{other.id_} {
        other.id_ = 0;
        other.driver_ = nullptr;
    }
    auto operator=(FakeHandle&& other) noexcept -> FakeHandle& {
        if (this == &other) {
            return *this;
        }

        reset();

        driver_ = other.driver_;
        id_ = other.id_;
        other.id_ = 0;
        other.driver_ = nullptr;

        return *this;
    }

    [[nodiscard]]
    auto is_valid() const noexcept -> bool {
        return id_ != 0;
    }

    auto reset() noexcept -> void {
        if (is_valid()) {
            fake_close(*driver_, id_);
            id_ = 0;
        }
    }

    [[nodiscard]]
    auto get() const noexcept -> std::uint32_t {
        assert(is_valid());
        return id_;
    }

  private:
    FakeDriver* driver_{};
    std::uint32_t id_{};
};

} // namespace engine::core
