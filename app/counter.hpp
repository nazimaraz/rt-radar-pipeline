#pragma once

#include <cstdint>

// Event counter that one task increments and any task may read.
class Counter {
public:
  void increment();

  [[nodiscard]] std::uint32_t value() const;

private:
  std::uint32_t value_{0};
};
