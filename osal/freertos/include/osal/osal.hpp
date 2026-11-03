#pragma once

// Thin OS abstraction over FreeRTOS. Application code uses only this header,
// so moving to another RTOS (VxWorks, an ARINC 653 partition OS, ...) means
// writing a new backend, not touching the pipeline.
//
// Every object owns its storage (task stack, queue buffer, control block):
// nothing is allocated at run time.

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

namespace osal {

using Milliseconds = std::uint32_t;
inline constexpr auto kForever{Milliseconds{0xFFFFFFFFu}};

[[nodiscard]] inline TickType_t toTicks(const Milliseconds ms) {
  return ms == kForever ? portMAX_DELAY : pdMS_TO_TICKS(ms);
}

// Milliseconds since the scheduler started; wraps after ~49 days.
[[nodiscard]] inline std::uint32_t nowMs() {
  return static_cast<std::uint32_t>(xTaskGetTickCount()) * portTICK_PERIOD_MS;
}

inline void sleepFor(const Milliseconds ms) { vTaskDelay(toTicks(ms)); }

// Drift-free periodic activation. Construct once in the task, call wait()
// at the top of each cycle.
class Periodic {
public:
  // clang-format off: the empty body goes on its own line
  explicit Periodic(const Milliseconds period)
    : period_{toTicks(period)}
    , lastWake_{xTaskGetTickCount()}
  {}
  // clang-format on

  // Blocks until the next period starts. Returns false when that start was
  // already in the past, i.e. the previous cycle overran its deadline.
  bool wait() { return xTaskDelayUntil(&lastWake_, period_) == pdTRUE; }

private:
  TickType_t period_;
  TickType_t lastWake_;
};

template <std::size_t StackWords> class Task {
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

// Fixed-capacity FIFO between tasks (or from an ISR to a task). Items are
// copied byte by byte, so T must be trivially copyable.
template <typename T, std::size_t Capacity> class Queue {
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

  // Returns false if the queue stayed full for the whole timeout.
  [[nodiscard]] bool send(const T& item, const Milliseconds timeout = 0) {
    return xQueueSend(handle_, &item, toTicks(timeout)) == pdPASS;
  }

  // Returns false if nothing arrived within the timeout.
  [[nodiscard]] bool receive(T& item, const Milliseconds timeout = kForever) {
    return xQueueReceive(handle_, &item, toTicks(timeout)) == pdPASS;
  }

  // ISR variant: never blocks. Pass the same flag to portYIELD_FROM_ISR().
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

// Mutual exclusion with priority inheritance: while a low-priority task holds
// the mutex, it runs at the priority of the highest task waiting for it.
class Mutex {
public:
  Mutex() = default;
  Mutex(const Mutex&) = delete;
  Mutex& operator=(const Mutex&) = delete;
  ~Mutex() = default;

  void create() {
    handle_ = xSemaphoreCreateMutexStatic(&control_);
    configASSERT(handle_ != nullptr);
  }
  [[nodiscard]] bool lock(const Milliseconds timeout = kForever) {
    return xSemaphoreTake(handle_, toTicks(timeout)) == pdPASS;
  }
  void unlock() { xSemaphoreGive(handle_); }

private:
  StaticSemaphore_t control_{};
  SemaphoreHandle_t handle_{nullptr};
};

// Signalling between tasks or from an ISR. No owner, so no priority
// inheritance: do not use it to protect a shared resource.
class BinarySemaphore {
public:
  BinarySemaphore() = default;
  BinarySemaphore(const BinarySemaphore&) = delete;
  BinarySemaphore& operator=(const BinarySemaphore&) = delete;
  ~BinarySemaphore() = default;

  void create() {
    handle_ = xSemaphoreCreateBinaryStatic(&control_);
    configASSERT(handle_ != nullptr);
  }
  [[nodiscard]] bool take(const Milliseconds timeout = kForever) {
    return xSemaphoreTake(handle_, toTicks(timeout)) == pdPASS;
  }
  void give() { xSemaphoreGive(handle_); }
  void giveFromIsr(BaseType_t& higherPriorityTaskWoken) {
    xSemaphoreGiveFromISR(handle_, &higherPriorityTaskWoken);
  }

private:
  StaticSemaphore_t control_{};
  SemaphoreHandle_t handle_{nullptr};
};

// RAII lock: unlocks when it goes out of scope, on every return path.
class LockGuard {
public:
  // clang-format off: the empty body goes on its own line
  explicit LockGuard(Mutex& mutex)
    : mutex_{mutex}
  {
    // kForever cannot time out, so the result carries no information.
    static_cast<void>(mutex_.lock());
  }
  // clang-format on
  ~LockGuard() { mutex_.unlock(); }
  LockGuard(const LockGuard&) = delete;
  LockGuard& operator=(const LockGuard&) = delete;

private:
  Mutex& mutex_;
};

} // namespace osal
