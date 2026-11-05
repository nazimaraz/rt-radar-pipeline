#include "heartbeat.hpp"

#include <cstdint>

#include "logger.hpp"

namespace heartbeat {
namespace {

constexpr auto kPeriod{osal::Milliseconds{1'000}};
auto task{osal::Task<256>{}};

void run(void*) {
  auto period{osal::Periodic{kPeriod}};
  auto beats{std::uint32_t{0}};
  for (;;) {
    const auto onTime{period.wait()};
    ++beats;
    logger::post("[", osal::nowMs(), " ms] heartbeat #", beats,
                 " | stack headroom ", task.stackHeadroomWords(),
                 " words | log dropped ", logger::droppedCount(),
                 onTime ? "" : " | OVERRUN", "\n");
  }
}

} // namespace

void start(const UBaseType_t priority) {
  task.start("heartbeat", priority, run);
}

} // namespace heartbeat
