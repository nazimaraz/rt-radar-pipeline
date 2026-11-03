#pragma once

#include <cstddef>
#include <cstdint>

#include "osal/osal.hpp"

namespace pipeline {

constexpr auto kBlockSamples{std::size_t{256}};
constexpr auto kBlockCount{std::size_t{4}};
static_assert(kBlockCount <= 255, "indices are uint8_t");

struct SampleBlock {
  std::uint32_t sequence;
  float samples[kBlockSamples];
};

class BlockPool {
public:
  using Index = std::uint8_t;

  struct Snapshot {
    std::size_t free;
    std::size_t full;
  };

  void create() {
    free_.create();
    full_.create();
    for (auto i{std::size_t{0}}; i < kBlockCount; ++i) {
      const auto sent{free_.send(static_cast<Index>(i))};
      configASSERT(sent);
    }
  }

  // Sampler side: never waits. False means the consumer holds every block.
  [[nodiscard]] bool acquire(Index& index) { return free_.receive(index, 0); }

  void publish(const Index index) {
    const auto sent{full_.send(index)};
    configASSERT(sent);
  }

  // Consumer side.
  [[nodiscard]] bool take(Index& index, const osal::Milliseconds timeout) {
    return full_.receive(index, timeout);
  }

  void release(const Index index) {
    const auto sent{free_.send(index)};
    configASSERT(sent);
  }

  [[nodiscard]] SampleBlock& block(const Index index) {
    configASSERT(index < kBlockCount);
    return blocks_[index];
  }

  // Blocks waiting in each queue, read at one instant. Every other block is
  // held by a task, so free + full + held must equal kBlockCount.
  [[nodiscard]] Snapshot snapshot() const {
    const osal::CriticalSection guard;
    return {free_.size(), full_.size()};
  }

private:
  osal::Queue<Index, kBlockCount> free_;
  osal::Queue<Index, kBlockCount> full_;
  SampleBlock blocks_[kBlockCount]{};
};

} // namespace pipeline
