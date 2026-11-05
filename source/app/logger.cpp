#include "logger.hpp"

#include "bsp/console.hpp"

namespace logger {
namespace {

constexpr auto kQueueDepth{std::size_t{16}};
constexpr auto kReportInterval{osal::Milliseconds{1'000}};

auto queue{osal::Queue<LogMessage, kQueueDepth>{}};
auto task{osal::Task<256>{}};
auto dropped{std::uint32_t{0u}};

[[nodiscard]] std::uint32_t loadDropped() {
  return __atomic_load_n(&dropped, __ATOMIC_RELAXED);
}

void update() {
  static auto reported{std::uint32_t{0u}};
  static auto message{LogMessage{}};
  if (queue.receive(message, kReportInterval)) {
    bsp::console::write(message.text);
  }

  if (const auto total = loadDropped(); total != reported) {
    reported = total;
    bsp::console::print("[log] ", total, " message(s) dropped so far\n");
  }
}

void run(void*) {
  for (;;) {
    update();
  }
}

} // namespace

void start(const UBaseType_t priority) {
  queue.create();
  task.start("logger", priority, run);
}

std::uint32_t droppedCount() { return loadDropped(); }

namespace detail {

BufferSink::BufferSink(char* const buffer, const std::size_t capacity)
  : buffer_{buffer}
  , capacity_{capacity}
{}

void BufferSink::put(const char c) {
  if (length_ + 1 < capacity_) {
    buffer_[length_++] = c;
  } else {
    truncated_ = true;
  }
}

void BufferSink::terminate() {
  if (truncated_) {
    buffer_[capacity_ - 3] = '~';
    buffer_[capacity_ - 2] = '\n';
    length_ = capacity_ - 1;
  }

  buffer_[length_] = '\0';
}

bool enqueue(const LogMessage& message) {
  if (queue.send(message, 0)) {
    return true;
  }

  __atomic_fetch_add(&dropped, 1u, __ATOMIC_RELAXED);
  return false;
}

} // namespace detail
} // namespace logger
