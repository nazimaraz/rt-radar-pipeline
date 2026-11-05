# rt-radar-pipeline

A hard real-time radar signal-processing pipeline on FreeRTOS, running bare metal on an
ARM Cortex-M4F (QEMU, MPS2-AN386). Written in C++23 under the constraints of
safety-critical embedded software: no heap, static allocation, rate-monotonic
priorities, measured deadlines and an OS abstraction layer.

```
rt-radar-pipeline 0.1.0 | FreeRTOS V11.3.3 | Cortex-M4F @ 25 MHz
[18 ms] block #0 dsp 1/1 ms | free 3 full 0 | lost 0 ovr 0 ddrop 0
[18 ms] #0 bin 24 (1500 Hz) power 4075.535 thr 27.491
[18 ms] #0 bin 80 (5000 Hz) power 991.375 thr 33.315
[1000 ms] heartbeat #1 | stack headroom 211 words | log dropped 0
```

## How it works

Every 16 ms the sampler fills a block with 256 samples from a simulated ADC (16 kHz,
noise and two sweeping tones) as 16-bit big-endian words, the byte order of VITA 49.
The processing task converts them to float, applies a Hann window, a radix-2 FFT and
CA-CFAR detection (Pfa = 10⁻⁶), and the reporter prints the targets found. Lost blocks,
deadline overruns, dropped results and buffer leaks are counted and reported.

```
 ┌──────────┐  block ids  ┌───────────────────┐  detections  ┌──────────┐  text   ┌────────┐
 │ Sampler  │ ──────────▶ │ Processing        │ ───────────▶ │ Reporter │ ──────▶ │ Logger │
 │ 16 ms    │   (queue)   │ Hann → FFT → CFAR │   (queue)    │          │ (queue) │ UART   │
 └──────────┘             └───────────────────┘              └──────────┘         └────────┘
      ▲                             │
      └─────── free block ids ──────┘   4 static blocks, passed by index, never copied
```

## Design

- **No heap.** `configSUPPORT_DYNAMIC_ALLOCATION` is 0 and the linker wraps `malloc`
  and `free`, so any heap use, including `new` or `std::vector`, fails the link.
- **No hidden runtime.** `-fno-exceptions -fno-rtti -fno-threadsafe-statics`,
  `-nostartfiles` and no syscall stubs. The only startup code is a hand-written
  `Reset_Handler`.
- **C++23 without a runtime.** `std::expected` for pool calls that can fail,
  `std::byteswap` for the ADC words, `std::to_underlying` for scoped priorities and
  `std::unreachable` after an exhaustive `switch`. All header-only: no heap, no
  exceptions, and no extra code at `-Os`.
- **Rate-monotonic priorities.** The shorter the period, the higher the priority. The
  logger runs lowest, owns the UART and drops messages instead of blocking real work.
- **OS abstraction layer.** Application code includes only `osal/osal.hpp`, so porting
  to VxWorks or an ARINC 653 partition OS means a new backend, not a new pipeline.
- **Single-precision FPU.** `-Wdouble-promotion` catches double math the M4F would run
  in software.
- **Small footprint.** The firmware needs about 18 KB of flash at `-Os` (28 KB in
  the Debug preset) and 22 KB of RAM.

## Build and run

Requirements: CMake 3.25+, Ninja, Arm GNU Toolchain (`arm-none-eabi`), QEMU with the
`mps2-an386` machine.

```bash
git clone --recursive https://github.com/nazimaraz/rt-radar-pipeline.git
cd rt-radar-pipeline

cmake --preset qemu && cmake --build --preset qemu
cmake --build --preset qemu --target run   # quit with Ctrl-A, then X
```

To debug, start `cmake --build --preset qemu --target debug-server` and attach with
`arm-none-eabi-gdb build/qemu/firmware.elf -ex "target remote :1234"`.

## Layout

```
source/app/     sampler, processing, reporter, logger and heartbeat tasks
source/dsp/     ADC format, FFT, Hann window, power spectrum and CA-CFAR: no OS
source/osal/    OS abstraction layer: osal.hpp picks the backend, freertos/ is one
source/bsp/     board API; mps2_an386/ has the startup code, linker script, UART
                driver and FreeRTOS config for this board
dependencies/   third-party libraries: FreeRTOS-Kernel V11.3.3 (git submodule)
cmake/          toolchain file and the QEMU run and debug targets
```

QEMU is not cycle-accurate: guest time follows host time. The timings here are good for
comparing designs, not for WCET claims.
