#pragma once

#include "osal/osal.hpp"

namespace app {

enum class Priority : UBaseType_t {
  kLogger = 1,
  kHeartbeat = 2,
  kReporter = 3,
  kProcessing = 4,
  kSampler = 5,
};

} // namespace app
