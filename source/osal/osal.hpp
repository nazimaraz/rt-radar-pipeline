#pragma once

// The only OSAL header application code includes, and the one place that picks
// the backend. A port to another OS adds osal/<os>/ with the same classes and
// changes these lines.
#include "osal/freertos/binary_semaphore.hpp"
#include "osal/freertos/critical_section.hpp"
#include "osal/freertos/lock_guard.hpp"
#include "osal/freertos/mutex.hpp"
#include "osal/freertos/periodic.hpp"
#include "osal/freertos/queue.hpp"
#include "osal/freertos/task.hpp"
#include "osal/freertos/time.hpp"
