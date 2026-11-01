#pragma once

namespace bsp {

// Brings up the board (console UART). Call once, first thing in main().
void init();

// Stops the emulator with the given exit code through ARM semihosting.
// On real hardware this is where a reset or a safe state would go.
[[noreturn]] void shutdown(int exitCode);

}  // namespace bsp
