# Board support for the MPS2 AN386 (Cortex-M4F): startup code, vector table,
# console UART, linker script and the FreeRTOS configuration for this board.
set(BOARD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mps2_an386)

add_library(bsp OBJECT
    ${BOARD_DIR}/startup.c
    ${BOARD_DIR}/bsp.cpp)
target_include_directories(bsp PUBLIC ${SOURCE_ROOT})
target_link_libraries(bsp PRIVATE project_warnings)

# FreeRTOSConfig.h for the kernel and for everything that includes its headers.
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config INTERFACE ${BOARD_DIR}/config)

# The executable links with this board's memory map.
set(BSP_LINKER_SCRIPT ${BOARD_DIR}/linker.ld PARENT_SCOPE)
