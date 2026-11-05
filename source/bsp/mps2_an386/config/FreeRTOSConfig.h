#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* FreeRTOS configuration for the Cortex-M4F on QEMU's MPS2 AN386.
 * Every kernel object is statically allocated: there is no FreeRTOS heap. */

/* Scheduling */
#define configCPU_CLOCK_HZ                         ( 25000000UL ) /* MPS2 system clock */
#define configTICK_RATE_HZ                         ( 1000 )       /* 1 ms tick */
#define configTICK_TYPE_WIDTH_IN_BITS              TICK_TYPE_WIDTH_32_BITS
#define configUSE_PREEMPTION                       1
#define configUSE_TIME_SLICING                     1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION    1 /* CLZ-based ready-list search */
#define configMAX_PRIORITIES                       8
#define configMINIMAL_STACK_SIZE                   ( 256 ) /* in words */
#define configMAX_TASK_NAME_LEN                    16
#define configIDLE_SHOULD_YIELD                    1

/* Memory: static allocation only. Creating a kernel object without
 * caller-provided storage does not link. */
#define configSUPPORT_STATIC_ALLOCATION            1
#define configSUPPORT_DYNAMIC_ALLOCATION           0

/* Synchronisation */
#define configUSE_MUTEXES                          1 /* mutexes use priority inheritance */
#define configUSE_RECURSIVE_MUTEXES                0
#define configUSE_COUNTING_SEMAPHORES              1
#define configQUEUE_REGISTRY_SIZE                  8
#define configUSE_TIMERS                           0

/* Hooks and diagnostics */
#define configUSE_IDLE_HOOK                        0
#define configUSE_TICK_HOOK                        0
#define configCHECK_FOR_STACK_OVERFLOW             2
#define configUSE_TRACE_FACILITY                   1
#define configRECORD_STACK_HIGH_ADDRESS            1

/* Interrupt priorities (lower number = more urgent). An ISR that calls a
 * ...FromISR() API must have a numeric priority >= configMAX_SYSCALL_INTERRUPT_PRIORITY.
 * 0xA0 is priority 5 with 3 implemented bits, and still valid with 4 or 8 bits.
 * ISRs more urgent than this are never masked by the kernel, so they must not
 * call any FreeRTOS API. */
#define configKERNEL_INTERRUPT_PRIORITY            255
#define configMAX_SYSCALL_INTERRUPT_PRIORITY       0xA0

/* Optional API */
#define INCLUDE_vTaskDelay                         1
#define INCLUDE_xTaskDelayUntil                    1
#define INCLUDE_vTaskPrioritySet                   1
#define INCLUDE_uxTaskPriorityGet                  1
#define INCLUDE_vTaskSuspend                       1
#define INCLUDE_uxTaskGetStackHighWaterMark        1
#define INCLUDE_xTaskGetSchedulerState             1
#define INCLUDE_xTaskGetCurrentTaskHandle          1

/* Assertions print the location and stop (osal_freertos.cpp). */
#ifdef __cplusplus
extern "C" {
#endif
void vAssertCalled( const char * file, int line ) __attribute__( ( noreturn ) );
#ifdef __cplusplus
}
#endif
#define configASSERT( x )    do { if( ( x ) == 0 ) { vAssertCalled( __FILE__, __LINE__ ); } } while( 0 )

#endif /* FREERTOS_CONFIG_H */
