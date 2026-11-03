#pragma once

#include "osal/osal.hpp"

namespace pipeline {

// Creates the buffer pool and starts the sampler and consumer tasks. Call once,
// after logger::start() and before the scheduler starts.
void start(UBaseType_t samplerPriority, UBaseType_t consumerPriority);

} // namespace pipeline
