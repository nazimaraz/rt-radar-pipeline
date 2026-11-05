# The firmware image: the pipeline tasks on top of the board, OS and DSP
# libraries. It lands in the top of the build tree: build/qemu/firmware.elf.
add_executable(firmware
    block_pool.cpp
    counter.cpp
    logger.cpp
    main.cpp
    pipeline.cpp
    signal_generator.cpp)
set_target_properties(firmware PROPERTIES
    SUFFIX .elf
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_compile_definitions(firmware PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(firmware PRIVATE bsp osal dsp project_warnings)
target_link_options(firmware PRIVATE
    -T${BSP_LINKER_SCRIPT}
    -Wl,-Map=$<TARGET_FILE_DIR:firmware>/firmware.map
    -Wl,--print-memory-usage
    # No heap: any use of malloc/free (including new, std::vector, ...)
    # becomes an undefined reference and fails the link.
    -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=free)
set_property(TARGET firmware APPEND PROPERTY LINK_DEPENDS ${BSP_LINKER_SCRIPT})

add_custom_command(TARGET firmware POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:firmware>)
