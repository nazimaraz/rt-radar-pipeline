#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "FreeRTOS.h"
#include "queue.h"

#include "osal/time.hpp"

namespace osal {

template <typename T, std::size_t Capacity>
class Queue {
  static_assert(std::is_trivially_copyable_v<T>,
                "FreeRTOS copies queue items with memcpy");

public:
  Queue() = default;
  Queue(const Queue&) = delete;
  Queue& operator=(const Queue&) = delete;
  ~Queue() = default;

  void create() {
    handle_ = xQueueCreateStatic(Capacity, sizeof(T), storage_, &control_);
    configASSERT(handle_ != nullptr);
  }

  [[nodiscard]] bool send(const T& item, const Milliseconds timeout = 0) {
    return xQueueSend(handle_, &item, toTicks(timeout)) == pdPASS;
  }

  [[nodiscard]] bool receive(T& item, const Milliseconds timeout = kForever) {
    return xQueueReceive(handle_, &item, toTicks(timeout)) == pdPASS;
  }

  [[nodiscard]] bool sendFromIsr(const T& item,
                                 BaseType_t& higherPriorityTaskWoken) {
    return xQueueSendFromISR(handle_, &item, &higherPriorityTaskWoken) ==
           pdPASS;
  }

  [[nodiscard]] std::size_t size() const {
    return uxQueueMessagesWaiting(handle_);
  }

private:
  alignas(T) std::uint8_t storage_[Capacity * sizeof(T)]{};
  StaticQueue_t control_{};
  QueueHandle_t handle_{nullptr};
};

} // namespace osal
