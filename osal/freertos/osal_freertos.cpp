#include "osal/osal.hpp"

#include "bsp/bsp.hpp"
#include "bsp/console.hpp"

namespace {

// With configSUPPORT_DYNAMIC_ALLOCATION == 0 the kernel cannot take the idle
// task's memory from a heap, so the application provides it.
auto idleTcb{StaticTask_t{}};
StackType_t idleStack[configMINIMAL_STACK_SIZE]{};

} // namespace

extern "C" {

void vApplicationGetIdleTaskMemory(StaticTask_t** const tcb,
                                   StackType_t** const stack,
                                   configSTACK_DEPTH_TYPE* const stackWords) {
  *tcb = &idleTcb;
  *stack = idleStack;
  *stackWords = configMINIMAL_STACK_SIZE;
}

void vApplicationStackOverflowHook(TaskHandle_t, char* const taskName) {
  taskDISABLE_INTERRUPTS();
  bsp::console::print("\n*** STACK OVERFLOW in task '", taskName, "' ***\n");
  bsp::shutdown(2);
}

void vAssertCalled(const char* const file, const int line) {
  taskDISABLE_INTERRUPTS();
  bsp::console::print("\n*** ASSERT FAILED at ", file, ":", line, " ***\n");
  bsp::shutdown(1);
}

} // extern "C"
