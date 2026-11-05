# Firmware build for the Cortex-M4F target: cmake --preset qemu

set(FREERTOS_DIR ${CMAKE_SOURCE_DIR}/external/FreeRTOS-Kernel)
set(BSP_DIR      ${CMAKE_SOURCE_DIR}/bsp/mps2_an386)

if(NOT EXISTS ${FREERTOS_DIR}/tasks.c)
    message(FATAL_ERROR "FreeRTOS-Kernel is missing: git submodule update --init")
endif()

# --- FreeRTOS kernel ----------------------------------------------------------
add_library(freertos_kernel STATIC
    ${FREERTOS_DIR}/tasks.c
    ${FREERTOS_DIR}/queue.c
    ${FREERTOS_DIR}/list.c
    ${FREERTOS_DIR}/timers.c
    ${FREERTOS_DIR}/portable/GCC/ARM_CM4F/port.c)
target_include_directories(freertos_kernel PUBLIC
    ${FREERTOS_DIR}/include
    ${FREERTOS_DIR}/portable/GCC/ARM_CM4F
    ${BSP_DIR}/config)

# --- Board support: startup, vector table, console UART ------------------------
add_library(bsp OBJECT
    ${BSP_DIR}/startup.c
    ${BSP_DIR}/bsp.cpp)
target_include_directories(bsp PUBLIC bsp/include)
target_link_libraries(bsp PRIVATE project_warnings)

# --- OS abstraction layer (FreeRTOS backend) ---------------------------------
add_library(osal OBJECT
    osal/freertos/osal/binary_semaphore.cpp
    osal/freertos/osal/critical_section.cpp
    osal/freertos/osal/lock_guard.cpp
    osal/freertos/osal/mutex.cpp
    osal/freertos/osal/osal_freertos.cpp
    osal/freertos/osal/periodic.cpp)
target_include_directories(osal PUBLIC osal/freertos)
target_link_libraries(osal PUBLIC freertos_kernel PRIVATE bsp project_warnings)

# --- Firmware -----------------------------------------------------------------
function(add_firmware name)
    add_executable(${name}
        app/block_pool.cpp
        app/counter.cpp
        app/logger.cpp
        app/main.cpp
        app/pipeline.cpp
        app/signal_generator.cpp)
    set_target_properties(${name} PROPERTIES SUFFIX .elf)
    target_compile_definitions(${name} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
    target_link_libraries(${name} PRIVATE bsp osal dsp project_warnings)
    target_link_options(${name} PRIVATE
        -T${BSP_DIR}/linker.ld
        -Wl,-Map=$<TARGET_FILE_DIR:${name}>/${name}.map
        -Wl,--print-memory-usage
        # No heap: any use of malloc/free (including new, std::vector, ...)
        # becomes an undefined reference and fails the link.
        -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=free)
    set_property(TARGET ${name} APPEND PROPERTY LINK_DEPENDS ${BSP_DIR}/linker.ld)
endfunction()

add_firmware(firmware)
add_custom_command(TARGET firmware POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:firmware>)

# --- Run and debug on QEMU ----------------------------------------------------
find_program(QEMU_ARM qemu-system-arm)
if(QEMU_ARM)
    set(QEMU_ARGS -machine mps2-an386 -cpu cortex-m4 -nographic
                  -semihosting-config enable=on,target=native)
    # Exit QEMU with Ctrl-A then X.
    add_custom_target(run
        COMMAND ${QEMU_ARM} ${QEMU_ARGS} -kernel $<TARGET_FILE:firmware>
        DEPENDS firmware USES_TERMINAL VERBATIM)
    # Halts at reset and waits for GDB on localhost:1234.
    add_custom_target(debug-server
        COMMAND ${QEMU_ARM} ${QEMU_ARGS} -kernel $<TARGET_FILE:firmware> -s -S
        DEPENDS firmware USES_TERMINAL VERBATIM)
endif()
