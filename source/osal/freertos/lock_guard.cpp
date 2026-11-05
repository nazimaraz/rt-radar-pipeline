#include "osal/freertos/lock_guard.hpp"

using namespace osal;

LockGuard::LockGuard(Mutex& mutex)
  : mutex_{mutex} {
  mutex_.lock();
}

LockGuard::~LockGuard() { mutex_.unlock(); }
