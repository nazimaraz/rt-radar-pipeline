# OS abstraction layer. osal.hpp picks the backend; this build uses freertos/.
add_library(osal OBJECT
    freertos/binary_semaphore.cpp
    freertos/critical_section.cpp
    freertos/hooks.cpp
    freertos/lock_guard.cpp
    freertos/mutex.cpp
    freertos/periodic.cpp)
target_include_directories(osal PUBLIC ${SOURCE_ROOT})
target_link_libraries(osal PUBLIC freertos_kernel PRIVATE bsp project_warnings)
