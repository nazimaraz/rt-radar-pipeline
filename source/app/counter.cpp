#include "counter.hpp"

void Counter::increment() { __atomic_fetch_add(&value_, 1u, __ATOMIC_RELAXED); }

std::uint32_t Counter::value() const {
  return __atomic_load_n(&value_, __ATOMIC_RELAXED);
}
