#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>

#include "osal/osal.hpp"

namespace pipeline {

constexpr auto kBlockSamples{std::size_t{256}};
constexpr auto kBlockCount{std::size_t{4}};
static_assert(kBlockCount <= 255, "indices are uint8_t");

struct SampleBlock {
  std::uint32_t sequence;
  std::uint16_t samples[kBlockSamples]; // raw ADC words, big-endian
};

enum class PoolError : std::uint8_t {
  kNoFreeBlock, // acquire(): the consumer holds every block
  kTimeout,     // take(): no full block arrived in time
};

[[nodiscard]] const char* describe(PoolError error);

class BlockPool {
public:
  using Index = std::uint8_t;

  struct Snapshot {
    std::size_t free;
    std::size_t full;
  };

  void create();

  // Sampler side: never waits.
  [[nodiscard]] std::expected<Index, PoolError> acquire();
  void publish(Index index);

  // Consumer side.
  [[nodiscard]] std::expected<Index, PoolError>
  take(osal::Milliseconds timeout);
  void release(Index index);

  [[nodiscard]] SampleBlock& block(Index index);

  // Blocks waiting in each queue, read at one instant. Every other block is
  // held by a task, so free + full + held must equal kBlockCount.
  [[nodiscard]] Snapshot snapshot() const;

private:
  osal::Queue<Index, kBlockCount> free_;
  osal::Queue<Index, kBlockCount> full_;
  SampleBlock blocks_[kBlockCount]{};
};

} // namespace pipeline
