#pragma once

#include "osal/osal.hpp"

namespace pipeline {

// Creates the buffer pool and the queues and starts the sampler, signal
// processing and reporter tasks. Call once, after logger::start() and before
// the scheduler starts.
void start(UBaseType_t samplerPriority, UBaseType_t processingPriority,
           UBaseType_t reporterPriority);

} // namespace pipeline
