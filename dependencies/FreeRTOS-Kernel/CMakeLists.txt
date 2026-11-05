set(FREERTOS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Kernel)

if(NOT EXISTS ${FREERTOS_DIR}/tasks.c)
    message(FATAL_ERROR "FreeRTOS-Kernel is missing: git submodule update --init")
endif()

add_library(freertos_kernel STATIC
    ${FREERTOS_DIR}/tasks.c
    ${FREERTOS_DIR}/queue.c
    ${FREERTOS_DIR}/list.c
    ${FREERTOS_DIR}/timers.c
    ${FREERTOS_DIR}/portable/GCC/ARM_CM4F/port.c)
target_include_directories(freertos_kernel PUBLIC
    ${FREERTOS_DIR}/include
    ${FREERTOS_DIR}/portable/GCC/ARM_CM4F)
target_link_libraries(freertos_kernel PUBLIC freertos_config)
