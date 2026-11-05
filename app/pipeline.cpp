#include "pipeline.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include "block_pool.hpp"
#include "counter.hpp"
#include "dsp/adc.hpp"
#include "dsp/cfar.hpp"
#include "dsp/fft.hpp"
#include "dsp/spectrum.hpp"
#include "dsp/window.hpp"
#include "logger.hpp"
#include "signal_generator.hpp"

namespace pipeline {
namespace {

static_assert(kBlockSamples == dsp::kFftSize,
              "one sample block is one FFT frame");

constexpr auto kBlockPeriod{osal::Milliseconds{
    (static_cast<std::uint32_t>(kBlockSamples) * 1'000u) / sim::kSampleRateHz}};
static_assert(kBlockPeriod * sim::kSampleRateHz ==
                  static_cast<std::uint32_t>(kBlockSamples) * 1'000u,
              "a block must span a whole number of milliseconds");

constexpr auto kFalseAlarmProbability{1e-6f};
constexpr auto kDetectionQueueDepth{std::size_t{4}};
constexpr auto kReportEveryBlocks{std::uint32_t{16}};
constexpr auto kReportedDetections{std::uint32_t{3}};

auto pool{BlockPool{}};
auto detections{osal::Queue<dsp::DetectionList, kDetectionQueueDepth>{}};
auto samplerTask{osal::Task<256>{}};
auto processingTask{osal::Task<384>{}};
auto reporterTask{osal::Task<256>{}};
auto lostBlocks{Counter{}};
auto samplerOverruns{Counter{}};
auto droppedLists{Counter{}};

const auto kWindow{dsp::HannWindow{}};
const auto kFft{dsp::Fft{}};
const auto kCfar{dsp::CaCfar{kFalseAlarmProbability}};

float decoded[kBlockSamples];
dsp::Complex spectrum[dsp::kFftSize];
float power[dsp::kSpectrumBins];

void sampler(void*) {
  auto period{osal::Periodic{kBlockPeriod}};
  auto generator{sim::SignalGenerator{}};
  auto sequence{std::uint32_t{0}};
  for (;;) {
    if (!period.wait()) {
      samplerOverruns.increment();
    }

    if (const auto acquired{pool.acquire()}) {
      auto& block{pool.block(*acquired)};
      block.sequence = sequence;
      generator.fill(block.samples, kBlockSamples, sequence);
      pool.publish(*acquired);
    } else {
      lostBlocks.increment();
    }
    ++sequence;
  }
}

void reportProcessing(const std::uint32_t sequence,
                      const osal::Milliseconds elapsed,
                      const osal::Milliseconds slowest,
                      const BlockPool::Snapshot& pooled, const bool leaked) {
  logger::post("[", osal::nowMs(), " ms] block #", sequence, " dsp ", elapsed,
               "/", slowest, " ms | free ", pooled.free, " full ", pooled.full,
               " | lost ", lostBlocks.value(), " ovr ", samplerOverruns.value(),
               " ddrop ", droppedLists.value(), leaked ? " LEAK" : "", "\n");
}

void processing(void*) {
  auto slowest{osal::Milliseconds{0}};
  for (;;) {
    const auto taken{pool.take(osal::kForever)};
    if (!taken) {
      logger::post("[", osal::nowMs(),
                   " ms] block pool: ", describe(taken.error()), "\n");
      continue;
    }

    const auto index{*taken};
    const auto started{osal::nowMs()};
    auto list{dsp::DetectionList{}};
    list.sequence = pool.block(index).sequence;
    dsp::decodeSamples(pool.block(index).samples, decoded, kBlockSamples);
    const auto pooled{pool.snapshot()};
    const auto leaked{pooled.free + pooled.full + 1u != kBlockCount};
    pool.release(index);
    kWindow.apply(decoded, spectrum);
    kFft.forward(spectrum);
    dsp::powerSpectrum(spectrum, power);
    list.count = kCfar.detect(power, list.items, dsp::kMaxDetections);
    if (!detections.send(list, 0)) {
      droppedLists.increment();
    }

    const auto elapsed{osal::nowMs() - started};
    slowest = std::max(slowest, elapsed);
    if (list.sequence % kReportEveryBlocks == 0u) {
      reportProcessing(list.sequence, elapsed, slowest, pooled, leaked);
    }
  }
}

void reportDetections(const dsp::DetectionList& list) {
  if (list.count == 0u) {
    logger::post("[", osal::nowMs(), " ms] #", list.sequence, " no target\n");
    return;
  }

  const auto shown{list.count < kReportedDetections ? list.count
                                                    : kReportedDetections};
  for (auto i{std::uint32_t{0}}; i < shown; ++i) {
    const auto& found{list.items[i]};
    const auto hertz{(std::uint32_t{found.bin} * sim::kSampleRateHz) /
                     static_cast<std::uint32_t>(dsp::kFftSize)};
    logger::post("[", osal::nowMs(), " ms] #", list.sequence, " bin ",
                 found.bin, " (", hertz, " Hz) power ", found.power, " thr ",
                 found.threshold, "\n");
  }
}

void reporter(void*) {
  auto list{dsp::DetectionList{}};
  for (;;) {
    if (!detections.receive(list, osal::kForever)) {
      continue;
    }

    if (list.sequence % kReportEveryBlocks == 0u) {
      reportDetections(list);
    }
  }
}

} // namespace

void start(const UBaseType_t samplerPriority,
           const UBaseType_t processingPriority,
           const UBaseType_t reporterPriority) {
  pool.create();
  detections.create();
  reporterTask.start("reporter", reporterPriority, reporter);
  processingTask.start("processing", processingPriority, processing);
  samplerTask.start("sampler", samplerPriority, sampler);
}

} // namespace pipeline
