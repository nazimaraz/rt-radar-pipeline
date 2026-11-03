#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

#include "osal/time.hpp"

namespace osal {

class Mutex {
public:
  Mutex();
  Mutex(const Mutex&) = delete;
  Mutex& operator=(const Mutex&) = delete;
  ~Mutex();

  void create();
  bool lock(Milliseconds timeout = kForever) const;
  void unlock() const;

private:
  StaticSemaphore_t control_{};
  SemaphoreHandle_t handle_{nullptr};
};

} // namespace osal
