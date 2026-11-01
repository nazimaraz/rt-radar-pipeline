#include "bsp/bsp.hpp"
#include "bsp/console.hpp"
#include "osal/osal.hpp"

namespace {

// Rate-monotonic order: the shorter a task's period, the higher its priority.
// Pipeline tasks get their priorities here as they are added.
enum Priority : UBaseType_t {
    kHeartbeatPriority = 1,
};

constexpr osal::Milliseconds kHeartbeatPeriod = 1000;

osal::Task<256> heartbeatTask;

// Proves the scheduler, the tick and periodic activation work. It also reports
// how much of its own stack has never been touched.
void heartbeat(void*) {
    osal::Periodic period(kHeartbeatPeriod);
    std::uint32_t beats = 0;
    for (;;) {
        const bool onTime = period.wait();
        ++beats;
        bsp::console::print("[", osal::nowMs(), " ms] heartbeat #", beats,
                            " | stack headroom ", heartbeatTask.stackHeadroomWords(),
                            " words", onTime ? "" : " | OVERRUN", "\n");
#ifdef RT_SMOKE_TEST_BEATS
        if (beats == RT_SMOKE_TEST_BEATS) {
            bsp::shutdown(0);  // boot smoke test passed
        }
#endif
    }
}

}  // namespace

int main() {
    bsp::init();
    bsp::console::print("rt-radar-pipeline ", PROJECT_VERSION, " | FreeRTOS ",
                        tskKERNEL_VERSION_NUMBER, " | Cortex-M4F @ ",
                        configCPU_CLOCK_HZ / 1000000u, " MHz\n");

    heartbeatTask.start("heartbeat", kHeartbeatPriority, heartbeat);

    vTaskStartScheduler();  // does not return while the system runs
    return 0;
}
