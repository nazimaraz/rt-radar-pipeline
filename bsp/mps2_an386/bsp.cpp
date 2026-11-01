#include "bsp/bsp.hpp"
#include "bsp/console.hpp"

#include <cstddef>
#include <cstdint>

namespace {

// CMSDK APB UART (ARM DDI 0479). QEMU connects UART0 to stdio with -nographic.
struct CmsdkUart {
    volatile std::uint32_t data;       // 0x00
    volatile std::uint32_t state;      // 0x04: bit 0 = TX buffer full
    volatile std::uint32_t ctrl;       // 0x08: bit 0 = TX enable
    volatile std::uint32_t intStatus;  // 0x0C
    volatile std::uint32_t bauddiv;    // 0x10
};
static_assert(offsetof(CmsdkUart, bauddiv) == 0x10, "register layout");

constexpr std::uintptr_t kUart0Base = 0x40004000u;
constexpr std::uint32_t kTxFull = 1u << 0;
constexpr std::uint32_t kTxEnable = 1u << 0;

CmsdkUart& uart0() { return *reinterpret_cast<CmsdkUart*>(kUart0Base); }

}  // namespace

namespace bsp {

void init() {
    uart0().bauddiv = 16;  // smallest legal divider; QEMU ignores baud timing
    uart0().ctrl = kTxEnable;
}

void shutdown(int exitCode) {
    // Semihosting SYS_EXIT_EXTENDED (0x20) with ADP_Stopped_ApplicationExit:
    // QEMU exits with exitCode. Needs -semihosting-config enable=on.
    const std::uint32_t block[2] = {0x20026u, static_cast<std::uint32_t>(exitCode)};
    asm volatile(
        "mov r0, #0x20\n\t"
        "mov r1, %0\n\t"
        "bkpt 0xAB"
        :
        : "r"(block)
        : "r0", "r1", "memory");
    for (;;) {
    }
}

}  // namespace bsp

namespace bsp::console {

void putChar(char c) {
    if (c == '\n') {
        putChar('\r');  // QEMU puts the terminal in raw mode
    }
    while ((uart0().state & kTxFull) != 0u) {
    }
    uart0().data = static_cast<std::uint8_t>(c);
}

void write(const char* text) {
    while (*text != '\0') {
        putChar(*text++);
    }
}

void writeUnsigned(std::uint32_t value, unsigned base) {
    if (base < 2u || base > 16u) {
        base = 10u;
    }
    char digits[32];
    std::size_t count = 0;
    do {
        digits[count++] = "0123456789abcdef"[value % base];
        value /= base;
    } while (value != 0u);
    while (count > 0u) {
        putChar(digits[--count]);
    }
}

void writeSigned(std::int32_t value) {
    if (value < 0) {
        putChar('-');
        writeUnsigned(0u - static_cast<std::uint32_t>(value));
    } else {
        writeUnsigned(static_cast<std::uint32_t>(value));
    }
}

void writeFloat(float value) {
    if (value != value) {
        write("nan");
        return;
    }
    if (value < 0.0f) {
        putChar('-');
        value = -value;
    }
    if (value >= 4.0e9f) {
        write("inf");
        return;
    }
    // 32-bit only: float -> uint32 is one VCVT instruction, while a 64-bit
    // conversion would pull in software double-precision routines.
    auto whole = static_cast<std::uint32_t>(value);
    auto fraction =
        static_cast<std::uint32_t>((value - static_cast<float>(whole)) * 1000.0f + 0.5f);
    if (fraction >= 1000u) {
        ++whole;
        fraction -= 1000u;
    }
    writeUnsigned(whole);
    putChar('.');
    if (fraction < 100u) {
        putChar('0');
    }
    if (fraction < 10u) {
        putChar('0');
    }
    writeUnsigned(fraction);
}

}  // namespace bsp::console

// Called by the fault vectors in startup.c.
extern "C" [[noreturn]] void bsp_fault(const char* what) {
    bsp::console::print("\n*** FAULT: ", what, " ***\n");
    bsp::shutdown(3);
}
