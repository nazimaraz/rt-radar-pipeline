# Cross-compilation toolchain for bare-metal ARM Cortex-M4F (Arm GNU Toolchain).
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Look in PATH first, then in the usual install locations.
set(_arm_hints "")
if(DEFINED ENV{ARM_GNU_TOOLCHAIN})
    list(APPEND _arm_hints "$ENV{ARM_GNU_TOOLCHAIN}/bin")
endif()
file(GLOB _arm_install_dirs
    "$ENV{HOME}/Toolchains/arm-gnu-toolchain-*/bin"
    "/Applications/ArmGNUToolchain/*/arm-none-eabi/bin")
list(APPEND _arm_hints ${_arm_install_dirs})

find_program(ARM_GCC arm-none-eabi-gcc HINTS ${_arm_hints} REQUIRED)
get_filename_component(_arm_bin "${ARM_GCC}" DIRECTORY)

set(CMAKE_C_COMPILER   "${_arm_bin}/arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "${_arm_bin}/arm-none-eabi-g++")
set(CMAKE_ASM_COMPILER "${_arm_bin}/arm-none-eabi-gcc")
set(CMAKE_SIZE         "${_arm_bin}/arm-none-eabi-size" CACHE FILEPATH "")
set(ARM_GDB            "${_arm_bin}/arm-none-eabi-gdb" CACHE FILEPATH "")

# Bare metal: configure checks cannot link or run a hosted executable.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(_mcu "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")
set(CMAKE_C_FLAGS_INIT   "${_mcu} -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS_INIT "${_mcu} -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit")
set(CMAKE_ASM_FLAGS_INIT "${_mcu}")
# -nostartfiles: Reset_Handler is the only startup code. newlib's crt0 would
# drag in exit(), stdio and malloc. No nosys.specs either, so any call that
# needs an OS service (write, sbrk, ...) fails the link instead of the run.
set(CMAKE_EXE_LINKER_FLAGS_INIT "${_mcu} -specs=nano.specs -nostartfiles -Wl,--gc-sections")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
