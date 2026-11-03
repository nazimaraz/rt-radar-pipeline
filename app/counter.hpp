#pragma once

#include <cstdint>

// Event counter that one task increments and any task may read.
class Counter {
public:
  void increment() { __atomic_fetch_add(&value_, 1u, __ATOMIC_RELAXED); }

  [[nodiscard]] std::uint32_t value() const {
    return __atomic_load_n(&value_, __ATOMIC_RELAXED);
  }

private:
  std::uint32_t value_{0};
};
