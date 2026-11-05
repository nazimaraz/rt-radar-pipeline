#pragma once

#include <cstddef>
#include <cstdint>

#include "bsp/format.hpp"
#include "osal/osal.hpp"

namespace logger {

constexpr auto kMaxTextLength{std::size_t{80}};

struct LogMessage {
  char text[kMaxTextLength];
};

void start(UBaseType_t priority);

[[nodiscard]] std::uint32_t droppedCount();

namespace detail {

class BufferSink {
public:
  BufferSink(char* buffer, std::size_t capacity);

  void put(char c);
  void terminate();

private:
  char* buffer_;
  std::size_t capacity_;
  std::size_t length_{0};
  bool truncated_{false};
};

bool enqueue(const LogMessage& message);

} // namespace detail

template <typename... Args>
void post(const Args&... args) {
  auto message{LogMessage{}};
  auto sink{detail::BufferSink{message.text, sizeof(message.text)}};
  bsp::format::print(sink, args...);
  sink.terminate();
  detail::enqueue(message);
}

} // namespace logger
