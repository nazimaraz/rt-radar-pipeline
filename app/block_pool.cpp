#include "block_pool.hpp"

namespace pipeline {

void BlockPool::create() {
  free_.create();
  full_.create();
  for (auto i{std::size_t{0}}; i < kBlockCount; ++i) {
    const auto sent{free_.send(static_cast<Index>(i))};
    configASSERT(sent);
  }
}

bool BlockPool::acquire(Index& index) { return free_.receive(index, 0); }

void BlockPool::publish(const Index index) {
  const auto sent{full_.send(index)};
  configASSERT(sent);
}

bool BlockPool::take(Index& index, const osal::Milliseconds timeout) {
  return full_.receive(index, timeout);
}

void BlockPool::release(const Index index) {
  const auto sent{free_.send(index)};
  configASSERT(sent);
}

SampleBlock& BlockPool::block(const Index index) {
  configASSERT(index < kBlockCount);
  return blocks_[index];
}

BlockPool::Snapshot BlockPool::snapshot() const {
  const osal::CriticalSection guard;
  return {.free = free_.size(), .full = full_.size()};
}

} // namespace pipeline
