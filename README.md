# rt-radar-pipeline

A hard real-time radar signal-processing pipeline on FreeRTOS, running bare metal on an
emulated ARM Cortex-M4F (QEMU, MPS2-AN386). Built to practise embedded real-time
software the way safety-critical projects do it: static memory, explicit priorities,
measured deadlines, an OS abstraction layer.

```
rt-radar-pipeline 0.1.0 | FreeRTOS V11.3.3 | Cortex-M4F @ 25 MHz
[1000 ms] heartbeat #1 | stack headroom 211 words
[2000 ms] heartbeat #2 | stack headroom 206 words
```

## Status

- [x] Bare-metal bring-up: reset handler, vector table, linker script, UART console
- [x] FreeRTOS 11.3.3, static allocation only, behind a thin OSAL
- [x] Periodic task with deadline-overrun detection and stack headroom report
- [x] Boot smoke test on QEMU (`ctest`)
- [x] Logger task fed by a queue (single UART owner, drops instead of blocking)
- [x] Signal generator (noise + two sweeping tones) with a zero-copy buffer pool
- [ ] FFT + CA-CFAR detection, unit tested on the host
- [ ] Alpha-beta tracker
- [ ] Timing and health monitoring (execution time, CPU load)
- [ ] Priority inversion demo: binary semaphore vs. mutex with priority inheritance
- [ ] Timer-interrupt-driven sampling

## Planned architecture

```
 TIMER0 IRQ
     │
     ▼
 ┌─────────┐  block ids   ┌──────────────┐  detections  ┌─────────┐  tracks  ┌────────┐
 │ Sampler │ ───────────▶ │ DSP          │ ───────────▶ │ Tracker │ ───────▶ │ Logger │
 │ highest │   (queue)    │ FFT + CFAR   │   (queue)    │         │ (queue)  │ lowest │
 └─────────┘              └──────────────┘              └─────────┘          └────────┘
      ▲                          │
      └──── free buffer ids ─────┘   static pool of sample buffers, no copies
```

Priorities follow rate-monotonic order: the shorter the period, the higher the priority.

## Design rules

- **No heap.** `configSUPPORT_DYNAMIC_ALLOCATION` is 0, so every task, queue and mutex
  owns its storage. The linker wraps `malloc`, `calloc`, `realloc` and `free`, so any heap
  use (including `new` or `std::vector`) fails the link instead of failing in the field.
- **No hidden runtime.** `-fno-exceptions -fno-rtti -fno-threadsafe-statics`, C++17.
  `-nostartfiles` and no syscall stubs: the only startup code is `Reset_Handler`, and
  anything that needs an OS service fails the link.
- **OS abstraction.** Application code includes only `osal/osal.hpp`. Porting to VxWorks
  or an ARINC 653 partition OS means a new `osal/<backend>/`, not a new pipeline.
- **Single-precision FPU.** `-Wdouble-promotion` flags accidental double-precision math,
  which the M4F would run in software.

## Layout

```
app/                  application tasks (main.cpp, logger)
bsp/include/bsp/      board API: init, shutdown, console, format
bsp/mps2_an386/       startup.c, linker.ld, UART driver, FreeRTOSConfig.h
osal/freertos/        OSAL backend: Task, Queue, Mutex, BinarySemaphore, Periodic
cmake/                arm-none-eabi toolchain file
external/             FreeRTOS-Kernel (git submodule, V11.3.3)
```

## Code style

The style follows `app/logger.*`: `auto x{T{}}` brace initialisation, `const` on
every parameter and local that is not reassigned (in definitions, not in
declarations), `[[nodiscard]]` on results that must be checked, trailing
underscore on private members, `kName` constants.

```bash
clang-format -i $(git ls-files '*.cpp' '*.hpp' '*.c')   # .clang-format
tools/clang-tidy.sh                                       # .clang-tidy, needs a configured build
```

`bsp/mps2_an386/config/` keeps FreeRTOS's own layout and is not reformatted.

## Build and run

Requirements: CMake 3.25+, Ninja, Arm GNU Toolchain (`arm-none-eabi`), QEMU with the
`mps2-an386` machine.

```bash
git clone --recursive <repo-url>
cmake --preset qemu
cmake --build --preset qemu
cmake --build --preset qemu --target run   # quit with Ctrl-A, then X
ctest --preset qemu                        # boot smoke test
```

In CLion, pick the **QEMU** run configuration (stored in `.run/`) and press Run. It
builds `firmware` and starts QEMU in the Run console. The `run` CMake target is for
the terminal only: in CLion's build window QEMU's output does not show and the build
never finishes.

The toolchain is looked up in `PATH`, `$ARM_GNU_TOOLCHAIN`,
`~/Toolchains/arm-gnu-toolchain-*` and `/Applications/ArmGNUToolchain/*`.

## Debugging

```bash
cmake --build --preset qemu --target debug-server   # halts at reset, GDB port 1234
arm-none-eabi-gdb build/qemu/firmware.elf -ex "target remote :1234"
```

## A note on timing

QEMU is not cycle-accurate: guest time follows host time. Timings measured here are
good for comparing designs (overruns, priority inversion delays), not for WCET claims.
