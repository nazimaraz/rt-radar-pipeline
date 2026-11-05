cmake_minimum_required(VERSION 3.25)
project(rt_radar_pipeline VERSION 0.1.0 LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Warnings for our own code. -Wdouble-promotion matters on the M4F: its FPU is
# single precision, so an accidental double is done in software.
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2)

# --- DSP: plain C++ with no OS, built for the target and for the host ---------
add_library(dsp STATIC
    dsp/dsp/adc.cpp
    dsp/dsp/cfar.cpp
    dsp/dsp/fft.cpp
    dsp/dsp/spectrum.cpp
    dsp/dsp/window.cpp)
target_include_directories(dsp PUBLIC dsp)
target_link_libraries(dsp PRIVATE project_warnings)

if(CMAKE_CROSSCOMPILING)
    include(cmake/firmware.cmake)
else()
    include(cmake/host.cmake)
endif()
