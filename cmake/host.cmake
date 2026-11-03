# Native build for the unit tests of the OS-independent code: cmake --preset host

enable_testing()

option(RT_HOST_SANITIZERS "Build the host tests with ASan and UBSan" ON)

include(FetchContent)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
    GIT_SHALLOW    TRUE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# The signal generator is simulation code in app/, but it has no OS dependency.
add_executable(dsp_tests
    app/signal_generator.cpp
    dsp/tests/cfar_test.cpp
    dsp/tests/chain_test.cpp
    dsp/tests/fft_test.cpp
    dsp/tests/window_test.cpp)
target_include_directories(dsp_tests PRIVATE app)
target_link_libraries(dsp_tests PRIVATE dsp GTest::gtest_main)

if(RT_HOST_SANITIZERS)
    target_compile_options(dsp_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(dsp_tests PRIVATE -fsanitize=address,undefined)
    target_compile_options(dsp PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
endif()

include(GoogleTest)
gtest_discover_tests(dsp_tests)
