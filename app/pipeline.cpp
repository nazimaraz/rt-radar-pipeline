#include "pipeline.hpp"

#include <cstddef>
#include <cstdint>

#include "block_pool.hpp"
#include "counter.hpp"
#include "logger.hpp"
#include "signal_generator.hpp"

namespace pipeline {
namespace {

constexpr auto kBlockPeriod{osal::Milliseconds{
    (static_cast<std::uint32_t>(kBlockSamples) * 1'000u) / sim::kSampleRateHz}};
static_assert(kBlockPeriod * sim::kSampleRateHz ==
                  static_cast<std::uint32_t>(kBlockSamples) * 1'000u,
              "a block must span a whole number of milliseconds");

constexpr auto kReportEveryBlocks{std::uint32_t{16}};

auto pool{BlockPool{}};
auto samplerTask{osal::Task<256>{}};
auto consumerTask{osal::Task<256>{}};

auto lostBlocks{Counter{}};
auto samplerOverruns{Counter{}};

[[nodiscard]] float meanPower(const float* const samples,
                              const std::size_t count) {
  auto sum{0.0f};
  for (auto i{std::size_t{0}}; i < count; ++i) {
    sum += samples[i] * samples[i];
  }
  return sum / static_cast<float>(count);
}

// Highest priority: one block every kBlockPeriod, no matter what. If the
// consumer holds every block, the new block is dropped and counted; the
// sampler never waits.
void sampler(void*) {
  auto period{osal::Periodic{kBlockPeriod}};
  auto generator{sim::SignalGenerator{}};
  auto sequence{std::uint32_t{0}};
  for (;;) {
    if (!period.wait()) {
      samplerOverruns.increment();
    }

    if (auto index{BlockPool::Index{0}}; pool.acquire(index)) {
      auto& block{pool.block(index)};
      block.sequence = sequence;
      generator.fill(block.samples, kBlockSamples, sequence);
      pool.publish(index);
    } else {
      lostBlocks.increment();
    }

    ++sequence;
  }
}

void report(const std::uint32_t sequence, const float power,
            const BlockPool::Snapshot& pooled, const bool leaked) {
  logger::post("[", osal::nowMs(), " ms] block #", sequence, " power ", power,
               " | free ", pooled.free, " full ", pooled.full, " | lost ",
               lostBlocks.value(), " ovr ", samplerOverruns.value(),
               leaked ? " | POOL LEAK" : "", "\n");
}

void consumer(void*) {
  for (;;) {
    auto index{BlockPool::Index{0}};
    if (!pool.take(index, osal::kForever)) {
      continue;
    }

    const auto& block{pool.block(index)};
    const auto sequence{block.sequence};
    const auto power{meanPower(block.samples, kBlockSamples)};
    const auto pooled{pool.snapshot()};
    const auto leaked{pooled.free + pooled.full + 1u != kBlockCount};
    pool.release(index);
    if (sequence % kReportEveryBlocks == 0u) {
      report(sequence, power, pooled, leaked);
    }
  }
}

} // namespace

void start(const UBaseType_t samplerPriority,
           const UBaseType_t consumerPriority) {
  pool.create();
  consumerTask.start("consumer", consumerPriority, consumer);
  samplerTask.start("sampler", samplerPriority, sampler);
}

} // namespace pipeline
