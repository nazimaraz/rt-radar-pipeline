# Our own code. source/ is the include root, so every header is included by its
# path from here: "bsp/bsp.hpp", "dsp/fft.hpp", "osal/osal.hpp".
set(SOURCE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Warnings for our own code. -Wdouble-promotion matters on the M4F: its FPU is
# single precision, so an accidental double is done in software.
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2)

add_subdirectory(bsp)
add_subdirectory(osal)
add_subdirectory(dsp)
add_subdirectory(app)
