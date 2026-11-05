#pragma once

#include "bsp/format.hpp"

// Heap-free console on UART0.
//
// Not thread-safe on purpose: two tasks printing at once interleave their
// characters. Once the scheduler runs, only the logger task writes here
// (app/logger.hpp). Everything else is single-threaded by construction:
// code before the scheduler starts, and the fault and assert handlers.
namespace bsp::console {

void putChar(char c);
void write(const char* text);

// Character sink that sends everything straight to the UART.
struct Sink {
  static void put(char c);
};

template <typename... Args>
void print(const Args&... args) {
  auto sink{Sink{}};
  format::print(sink, args...);
}

} // namespace bsp::console
