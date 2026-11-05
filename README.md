# rt-radar-pipeline

A hard real-time radar signal-processing pipeline on FreeRTOS, running bare metal on an
ARM Cortex-M4F (QEMU, MPS2-AN386). Written in C++23 under the constraints of
safety-critical embedded software: no heap, static allocation, rate-monotonic
priorities, measured deadlines and an OS abstraction layer.

```
rt-radar-pipeline 0.1.0 | FreeRTOS V11.3.3 | Cortex-M4F @ 25 MHz
[17 ms] block #0 dsp 0/0 ms | free 3 full 0 | lost 0 ovr 0 ddrop 0
[18 ms] #0 bin 24 (1500 Hz) power 4075.825 thr 27.495
[18 ms] #0 bin 80 (5000 Hz) power 991.403 thr 33.315
[1000 ms] heartbeat #1 | stack headroom 211 words | log dropped 0
```

## How it works

Every 16 ms the sampler fills a 256-sample block (16 kHz, simulated noise and two
sweeping tones). The processing task applies a Hann window, a radix-2 FFT and CA-CFAR
detection (Pfa = 10⁻⁶), and the reporter prints the targets found. Lost blocks,
deadline overruns, dropped results and buffer leaks are counted, and the QEMU smoke
test fails if any of them is non-zero.

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
- **Rate-monotonic priorities.** The shorter the period, the higher the priority. The
  logger runs lowest, owns the UART and drops messages instead of blocking real work.
- **OS abstraction layer.** Application code includes only `osal/osal.hpp`, so porting
  to VxWorks or an ARINC 653 partition OS means a new backend, not a new pipeline.
- **Single-precision FPU.** `-Wdouble-promotion` catches double math the M4F would run
  in software.
- **Tested.** 16 GoogleTest cases cover the DSP on the host, and `ctest` boots the
  firmware in QEMU. The firmware takes about 27 KB of flash and 23 KB of RAM.

## Build and run

Requirements: CMake 3.25+, Ninja, Arm GNU Toolchain (`arm-none-eabi`), QEMU with the
`mps2-an386` machine, and a host C++ compiler for the unit tests.

```bash
git clone --recursive https://github.com/nazimaraz/rt-radar-pipeline.git
cd rt-radar-pipeline

cmake --preset qemu && cmake --build --preset qemu
cmake --build --preset qemu --target run   # quit with Ctrl-A, then X
ctest --preset qemu                        # boot smoke test

cmake --preset host && cmake --build --preset host
ctest --preset host                        # DSP unit tests
```

To debug, start `cmake --build --preset qemu --target debug-server` and attach with
`arm-none-eabi-gdb build/qemu/firmware.elf -ex "target remote :1234"`.

## Layout

```
app/     sampler, processing, reporter, logger and heartbeat tasks
dsp/     FFT, Hann window, power spectrum, CA-CFAR (no OS) and their unit tests
osal/    OS abstraction layer with a FreeRTOS backend
bsp/     startup code, linker script, UART driver and FreeRTOS config for MPS2-AN386
cmake/   toolchain file and build scripts
```

QEMU is not cycle-accurate: guest time follows host time. The timings here are good for
comparing designs, not for WCET claims.
