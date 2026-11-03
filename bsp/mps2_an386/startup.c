/* Reset handler and vector table for the Cortex-M4F on MPS2 AN386. */

#include <stdint.h>

extern uint32_t __etext;
extern uint32_t __data_start__, __data_end__;
extern uint32_t __bss_start__, __bss_end__;
extern uint32_t __StackTop;

typedef void (*init_fn_t)(void);
extern init_fn_t __preinit_array_start[], __preinit_array_end[];
extern init_fn_t __init_array_start[], __init_array_end[];

extern int main(void);
extern void bsp_fault(const char* what) __attribute__((noreturn));

/* FreeRTOS port handlers go straight into the vector table ("direct routing"). */
extern void vPortSVCHandler(void);
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);

#define SCB_CPACR (*(volatile uint32_t*)0xE000ED88u)

void Reset_Handler(void) {
  /* Enable the FPU (full access to CP10 and CP11) before any float instruction. */
  SCB_CPACR |= (0xFu << 20);
  __asm volatile("dsb\n\tisb" ::: "memory");

  const uint32_t* src = &__etext;
  for (uint32_t* dst = &__data_start__; dst < &__data_end__;) {
    *dst++ = *src++;
  }
  for (uint32_t* dst = &__bss_start__; dst < &__bss_end__;) {
    *dst++ = 0u;
  }

  /* C++ static constructors */
  for (init_fn_t* fn = __preinit_array_start; fn < __preinit_array_end; ++fn) {
    (*fn)();
  }
  for (init_fn_t* fn = __init_array_start; fn < __init_array_end; ++fn) {
    (*fn)();
  }

  (void)main();
  bsp_fault("main() returned");
}

static void NMI_Handler(void) { bsp_fault("NMI"); }
static void HardFault_Handler(void) { bsp_fault("HardFault"); }
static void MemManage_Handler(void) { bsp_fault("MemManage"); }
static void BusFault_Handler(void) { bsp_fault("BusFault"); }
static void UsageFault_Handler(void) { bsp_fault("UsageFault"); }

void Default_Handler(void) { bsp_fault("unexpected interrupt"); }

/* Device interrupts. Define a function with the same name to handle one.
 * IRQ numbers follow the AN386 image (and QEMU hw/arm/mps2.c). */
void TIMER0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIMER1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

typedef void (*vector_t)(void);

#define IRQ(n) (16 + (n))

/* clang-format off */
__attribute__((section(".isr_vector"), used))
const vector_t vector_table[IRQ(32)] = {
    [0] = (vector_t)&__StackTop,
    [1] = Reset_Handler,
    [2] = NMI_Handler,
    [3] = HardFault_Handler,
    [4] = MemManage_Handler,
    [5] = BusFault_Handler,
    [6] = UsageFault_Handler,
    [11] = vPortSVCHandler,
    [12] = Default_Handler, /* DebugMon */
    [14] = xPortPendSVHandler,
    [15] = xPortSysTickHandler,
    [IRQ(0) ... IRQ(7)] = Default_Handler,
    [IRQ(8)] = TIMER0_IRQHandler,
    [IRQ(9)] = TIMER1_IRQHandler,
    [IRQ(10) ... IRQ(31)] = Default_Handler,
};
/* clang-format on */
