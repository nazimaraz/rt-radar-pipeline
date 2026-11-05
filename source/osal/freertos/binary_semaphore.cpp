#include "osal/freertos/binary_semaphore.hpp"

using namespace osal;

BinarySemaphore::BinarySemaphore() = default;

BinarySemaphore::~BinarySemaphore() = default;

void BinarySemaphore::create() {
  handle_ = xSemaphoreCreateBinaryStatic(&control_);
  configASSERT(handle_ != nullptr);
}

bool BinarySemaphore::take(const Milliseconds timeout) const {
  return xSemaphoreTake(handle_, toTicks(timeout)) == pdPASS;
}

void BinarySemaphore::give() const { xSemaphoreGive(handle_); }

void BinarySemaphore::giveFromIsr(BaseType_t& higherPriorityTaskWoken) const {
  xSemaphoreGiveFromISR(handle_, &higherPriorityTaskWoken);
}
