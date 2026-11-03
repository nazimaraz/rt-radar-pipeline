#include <cstdint>

#include "bsp/bsp.hpp"
#include "logger.hpp"
#include "osal/osal.hpp"
#include "pipeline.hpp"

namespace {

// Rate-monotonic order: the shorter a task's period, the higher its priority.
// Pipeline tasks get their priorities here as they are added.
enum Priority : UBaseType_t {
  kLoggerPriority = 1, // lowest of ours: logging must never delay real work
  kHeartbeatPriority = 2,
  kConsumerPriority = 3,
  kSamplerPriority = 4, // 16 ms period: the shortest, so the highest
};

constexpr auto kHeartbeatPeriod{osal::Milliseconds{1'000}};

auto heartbeatTask{osal::Task<256>{}};

// Proves the scheduler, the tick and periodic activation work. It also reports
// how much of its own stack has never been touched.
void heartbeat(void*) {
  auto period{osal::Periodic{kHeartbeatPeriod}};
  auto beats{std::uint32_t{0}};
  for (;;) {
    const auto onTime{period.wait()};
    ++beats;
    logger::post("[", osal::nowMs(), " ms] heartbeat #", beats,
                 " | stack headroom ", heartbeatTask.stackHeadroomWords(),
                 " words | log dropped ", logger::droppedCount(),
                 onTime ? "" : " | OVERRUN", "\n");
#ifdef RT_SMOKE_TEST_BEATS
    if (beats == RT_SMOKE_TEST_BEATS) {
      osal::sleepFor(20); // let the logger print the last line first
      bsp::shutdown(0);   // boot smoke test passed
    }
#endif
  }
}

} // namespace

int main() {
  bsp::init();
  logger::start(kLoggerPriority);
  logger::post("rt-radar-pipeline ", PROJECT_VERSION, " | FreeRTOS ",
               tskKERNEL_VERSION_NUMBER, " | Cortex-M4F @ ",
               configCPU_CLOCK_HZ / 1000000u, " MHz\n");

  pipeline::start(kSamplerPriority, kConsumerPriority);
  heartbeatTask.start("heartbeat", kHeartbeatPriority, heartbeat);

  vTaskStartScheduler(); // does not return while the system runs
  return 0;
}
