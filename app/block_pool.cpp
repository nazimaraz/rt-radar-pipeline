#include "block_pool.hpp"

#include <utility>

namespace pipeline {

const char* describe(const PoolError error) {
  switch (error) {
  case PoolError::kNoFreeBlock:
    return "no free block";
  case PoolError::kTimeout:
    return "timed out waiting for a block";
  }
  std::unreachable(); // every enumerator returns above
}

void BlockPool::create() {
  free_.create();
  full_.create();
  for (auto i{std::size_t{0}}; i < kBlockCount; ++i) {
    const auto sent{free_.send(static_cast<Index>(i))};
    configASSERT(sent);
  }
}

std::expected<BlockPool::Index, PoolError> BlockPool::acquire() {
  auto index{Index{0}};
  if (!free_.receive(index, 0)) {
    return std::unexpected{PoolError::kNoFreeBlock};
  }
  return index;
}

void BlockPool::publish(const Index index) {
  const auto sent{full_.send(index)};
  configASSERT(sent);
}

std::expected<BlockPool::Index, PoolError>
BlockPool::take(const osal::Milliseconds timeout) {
  auto index{Index{0}};
  if (!full_.receive(index, timeout)) {
    return std::unexpected{PoolError::kTimeout};
  }
  return index;
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
