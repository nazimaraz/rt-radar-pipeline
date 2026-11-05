#include "bsp/bsp.hpp"
#include "bsp/console.hpp"

#include <cstddef>
#include <cstdint>

namespace {

// CMSDK APB UART (ARM DDI 0479). QEMU connects UART0 to stdio with -nographic.
struct CmsdkUart {
  volatile std::uint32_t data;      // 0x00
  volatile std::uint32_t state;     // 0x04: bit 0 = TX buffer full
  volatile std::uint32_t ctrl;      // 0x08: bit 0 = TX enable
  volatile std::uint32_t intStatus; // 0x0C
  volatile std::uint32_t bauddiv;   // 0x10
};
static_assert(offsetof(CmsdkUart, bauddiv) == 0x10, "register layout");

constexpr auto kUart0Base{std::uintptr_t{0x40004000u}};
constexpr auto kTxFull{std::uint32_t{1u << 0u}};
constexpr auto kTxEnable{std::uint32_t{1u << 0u}};

[[nodiscard]] CmsdkUart& uart0() {
  return *reinterpret_cast<CmsdkUart*>(kUart0Base);
}

} // namespace

namespace bsp {

void init() {
  uart0().bauddiv = 16; // smallest legal divider; QEMU ignores baud timing
  uart0().ctrl = kTxEnable;
}

void shutdown(const int exitCode) {
  // Semihosting SYS_EXIT_EXTENDED (0x20) with ADP_Stopped_ApplicationExit:
  // QEMU exits with exitCode. Needs -semihosting-config enable=on.
  const std::uint32_t block[2]{0x20026u, static_cast<std::uint32_t>(exitCode)};
  asm volatile("mov r0, #0x20\n\t"
               "mov r1, %0\n\t"
               "bkpt 0xAB"
               :
               : "r"(block)
               : "r0", "r1", "memory");
  for (;;) {
  }
}

} // namespace bsp

namespace bsp::console {

void putChar(const char c) {
  if (c == '\n') {
    putChar('\r'); // QEMU puts the terminal in raw mode
  }
  while ((uart0().state & kTxFull) != 0u) {
  }
  uart0().data = static_cast<std::uint8_t>(c);
}

void Sink::put(const char c) { putChar(c); }

void write(const char* text) {
  while (*text != '\0') {
    putChar(*text++);
  }
}

} // namespace bsp::console

// Called by the fault vectors in startup.c.
// NOLINTNEXTLINE(readability-identifier-naming): C symbol called by startup.c
extern "C" [[noreturn]] void bsp_fault(const char* const what) {
  bsp::console::print("\n*** FAULT: ", what, " ***\n");
  bsp::shutdown(3);
}
