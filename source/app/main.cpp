#include <utility>

#include "bsp/bsp.hpp"
#include "heartbeat.hpp"
#include "logger.hpp"
#include "osal/osal.hpp"
#include "pipeline.hpp"
#include "priority.hpp"

namespace {

using app::Priority;

} // namespace

int main() {
  bsp::init();
  logger::start(std::to_underlying(Priority::kLogger));
  logger::post("rt-radar-pipeline ", PROJECT_VERSION, " | FreeRTOS ",
               tskKERNEL_VERSION_NUMBER, " | Cortex-M4F @ ",
               configCPU_CLOCK_HZ / 1000000u, " MHz\n");
  pipeline::start(std::to_underlying(Priority::kSampler),
                  std::to_underlying(Priority::kProcessing),
                  std::to_underlying(Priority::kReporter));
  heartbeat::start(std::to_underlying(Priority::kHeartbeat));
  vTaskStartScheduler();
  return 0;
}
