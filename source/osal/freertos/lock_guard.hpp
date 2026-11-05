#pragma once

#include "osal/freertos/mutex.hpp"

namespace osal {

class LockGuard {
public:
  explicit LockGuard(Mutex& mutex);
  ~LockGuard();
  LockGuard(const LockGuard&) = delete;
  LockGuard& operator=(const LockGuard&) = delete;

private:
  Mutex& mutex_;
};

} // namespace osal
