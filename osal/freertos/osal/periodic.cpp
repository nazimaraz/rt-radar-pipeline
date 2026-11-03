#include "osal/periodic.hpp"

using namespace osal;

Periodic::Periodic(const Milliseconds period)
  : period_{toTicks(period)}
  , lastWake_{xTaskGetTickCount()} {}

bool Periodic::wait() { return xTaskDelayUntil(&lastWake_, period_) == pdTRUE; }
