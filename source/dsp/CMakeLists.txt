# Signal processing: plain C++ with no OS dependency.
add_library(dsp STATIC
    adc.cpp
    cfar.cpp
    fft.cpp
    spectrum.cpp
    window.cpp)
target_include_directories(dsp PUBLIC ${SOURCE_ROOT})
target_link_libraries(dsp PRIVATE project_warnings)
