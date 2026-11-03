#pragma once

#include "FreeRTOS.h"
#include "osal/time.hpp"

namespace osal {

class Periodic {
public:
  explicit Periodic(Milliseconds period);

  bool wait();

private:
  TickType_t period_;
  TickType_t lastWake_;
};

} // namespace osal
