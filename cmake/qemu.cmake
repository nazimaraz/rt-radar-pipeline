# Run and debug the firmware on QEMU's MPS2 AN386 model.
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
