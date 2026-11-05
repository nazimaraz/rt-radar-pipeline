cmake_minimum_required(VERSION 3.25)
project(rt_radar_pipeline VERSION 0.1.0 LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CROSSCOMPILING)
    message(FATAL_ERROR "Only the firmware build exists: cmake --preset qemu")
endif()

add_subdirectory(dependencies)
add_subdirectory(source)
include(cmake/qemu.cmake)
