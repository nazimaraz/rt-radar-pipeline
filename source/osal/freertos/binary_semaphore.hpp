#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

#include "osal/freertos/time.hpp"

namespace osal {

class BinarySemaphore {
public:
  BinarySemaphore();
  BinarySemaphore(const BinarySemaphore&) = delete;
  BinarySemaphore& operator=(const BinarySemaphore&) = delete;
  ~BinarySemaphore();

  void create();
  [[nodiscard]] bool take(Milliseconds timeout = kForever) const;
  void give() const;
  void giveFromIsr(BaseType_t& higherPriorityTaskWoken) const;

private:
  StaticSemaphore_t control_{};
  SemaphoreHandle_t handle_{nullptr};
};

} // namespace osal
