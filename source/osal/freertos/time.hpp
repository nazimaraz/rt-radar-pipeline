#pragma once

#include <cstdint>

#include "FreeRTOS.h"
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

} // namespace osal
