#include "osal/freertos/critical_section.hpp"

#include "FreeRTOS.h"
#include "task.h"

using namespace osal;

CriticalSection::CriticalSection() { taskENTER_CRITICAL(); }

CriticalSection::~CriticalSection() { taskEXIT_CRITICAL(); }
