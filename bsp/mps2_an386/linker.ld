/* Memory map of the MPS2 AN386 FPGA image (Cortex-M4F) as emulated by QEMU. */

ENTRY(Reset_Handler)

MEMORY
{
    CODE (rx)  : ORIGIN = 0x00000000, LENGTH = 4M   /* SSRAM1, used as flash */
    RAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 4M   /* SSRAM2/3 */
}

/* Main stack: used by main() until the scheduler starts, then by every ISR.
 * Each task has its own stack, statically allocated in .bss. */
__main_stack_size = 4K;

SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)

        . = ALIGN(4);
        __preinit_array_start = .;
        KEEP(*(.preinit_array))
        __preinit_array_end = .;

        . = ALIGN(4);
        __init_array_start = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end = .;

        KEEP(*(.init))
        KEEP(*(.fini))
    } > CODE

    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > CODE

    . = ALIGN(4);
    __etext = .;

    /* Initial values live in CODE; Reset_Handler copies them to RAM. */
    .data : AT(__etext)
    {
        __data_start__ = .;
        *(.data*)
        . = ALIGN(4);
        __data_end__ = .;
    } > RAM

    .bss (NOLOAD) :
    {
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM

    .stack (NOLOAD) :
    {
        . = ALIGN(8);
        __StackLimit = .;
        . += __main_stack_size;
        __StackTop = .;
    } > RAM
}
