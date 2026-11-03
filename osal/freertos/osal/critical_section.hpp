#pragma once

namespace osal {

class CriticalSection {
public:
  CriticalSection();
  ~CriticalSection();
  CriticalSection(const CriticalSection&) = delete;
  CriticalSection& operator=(const CriticalSection&) = delete;
};

} // namespace osal
