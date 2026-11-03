#include "osal/mutex.hpp"

using namespace osal;

Mutex::Mutex() = default;

Mutex::~Mutex() = default;

void Mutex::create() {
  handle_ = xSemaphoreCreateMutexStatic(&control_);
  configASSERT(handle_ != nullptr);
}

bool Mutex::lock(const Milliseconds timeout) const {
  return xSemaphoreTake(handle_, toTicks(timeout)) == pdPASS;
}

void Mutex::unlock() const { xSemaphoreGive(handle_); }
