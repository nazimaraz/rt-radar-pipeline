#pragma once

#include <cstddef>

#include "FreeRTOS.h"
#include "task.h"

namespace osal {

template <std::size_t StackWords>
class Task {
public:
  using Entry = void (*)(void*);

  Task() = default;
  Task(const Task&) = delete;
  Task& operator=(const Task&) = delete;
  ~Task() = default;

  // Higher number = higher priority, up to configMAX_PRIORITIES - 1.
  void start(const char* const name, const UBaseType_t priority,
             const Entry entry, void* arg = nullptr) {
    configASSERT(priority < configMAX_PRIORITIES);
    handle_ = xTaskCreateStatic(entry, name, StackWords, arg, priority, stack_,
                                &tcb_);
    configASSERT(handle_ != nullptr);
  }

  // Smallest amount of stack that has stayed unused so far, in words.
  [[nodiscard]] std::size_t stackHeadroomWords() const {
    return uxTaskGetStackHighWaterMark(handle_);
  }
  [[nodiscard]] TaskHandle_t handle() const { return handle_; }

private:
  StackType_t stack_[StackWords]{};
  StaticTask_t tcb_{};
  TaskHandle_t handle_{nullptr};
};

} // namespace osal
