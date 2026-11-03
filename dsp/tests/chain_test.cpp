#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include "dsp/cfar.hpp"
#include "dsp/fft.hpp"
#include "dsp/spectrum.hpp"
#include "dsp/window.hpp"
#include "signal_generator.hpp"

namespace {

constexpr auto kBinWidthHz =
    static_cast<float>(sim::kSampleRateHz) / dsp::kFftSize;

// The whole receive chain on a simulated block, as the firmware runs it.
std::vector<dsp::Detection> process(const std::vector<float>& samples) {
  static const dsp::HannWindow window;
  static const dsp::Fft fft;
  static const dsp::CaCfar cfar{1e-6f};

  auto buffer = std::vector<dsp::Complex>(dsp::kFftSize);
  auto power = std::vector<float>(dsp::kSpectrumBins);
  window.apply(samples.data(), buffer.data());
  fft.forward(buffer.data());
  dsp::powerSpectrum(buffer.data(), power.data());

  dsp::Detection found[dsp::kMaxDetections];
  const auto count = cfar.detect(power.data(), found, dsp::kMaxDetections);
  return {found, found + count};
}

bool hasDetectionNear(const std::vector<dsp::Detection>& detections,
                      const float hertz) {
  const auto expected = hertz / kBinWidthHz;
  return std::any_of(
      detections.begin(), detections.end(), [&](const auto& detection) {
        return std::abs(static_cast<float>(detection.bin) - expected) <= 1.0f;
      });
}

} // namespace

TEST(Chain, FindsBothSweepingTargetsInEveryBlock) {
  sim::SignalGenerator generator;
  auto samples = std::vector<float>(dsp::kFftSize);

  for (auto block = 0u; block < 100u; ++block) {
    generator.fill(samples.data(), samples.size(), block);
    const auto detections = process(samples);

    const auto targetA = 1500.0f + (10.0f * static_cast<float>(block));
    const auto targetB = 5000.0f - (8.0f * static_cast<float>(block));
    EXPECT_TRUE(hasDetectionNear(detections, targetA)) << "block " << block;
    EXPECT_TRUE(hasDetectionNear(detections, targetB)) << "block " << block;
    EXPECT_EQ(detections.size(), 2u) << "block " << block;
  }
}
