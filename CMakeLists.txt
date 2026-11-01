cmake_minimum_required(VERSION 3.25)
project(rt_radar_pipeline VERSION 0.1.0 LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CROSSCOMPILING)
    message(FATAL_ERROR
        "Only the firmware build exists so far: cmake --preset qemu\n"
        "The host build for unit tests comes later.")
endif()

set(FREERTOS_DIR ${CMAKE_SOURCE_DIR}/external/FreeRTOS-Kernel)
set(BSP_DIR      ${CMAKE_SOURCE_DIR}/bsp/mps2_an386)

if(NOT EXISTS ${FREERTOS_DIR}/tasks.c)
    message(FATAL_ERROR "FreeRTOS-Kernel is missing: git submodule update --init")
endif()

# Warnings for our own code. -Wdouble-promotion matters on the M4F: its FPU is
# single precision, so an accidental double is done in software.
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2)

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
add_library(osal OBJECT osal/freertos/osal_freertos.cpp)
target_include_directories(osal PUBLIC osal/freertos/include)
target_link_libraries(osal PUBLIC freertos_kernel PRIVATE bsp project_warnings)

# --- Firmware -----------------------------------------------------------------
function(add_firmware name)
    add_executable(${name} app/main.cpp)
    set_target_properties(${name} PROPERTIES SUFFIX .elf)
    target_compile_definitions(${name} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
    target_link_libraries(${name} PRIVATE bsp osal project_warnings)
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

# Boot smoke test: the same firmware, built to exit QEMU with status 0 after
# three heartbeats. Faults and failed asserts exit with a non-zero status.
add_firmware(firmware_smoke)
target_compile_definitions(firmware_smoke PRIVATE RT_SMOKE_TEST_BEATS=3)

# --- Run, debug and test on QEMU ----------------------------------------------
enable_testing()
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

    add_test(NAME boot_smoke
        COMMAND ${QEMU_ARM} ${QEMU_ARGS} -kernel $<TARGET_FILE:firmware_smoke>)
    set_tests_properties(boot_smoke PROPERTIES
        TIMEOUT 10
        FAIL_REGULAR_EXPRESSION "\\*\\*\\*|OVERRUN")
endif()
