#include <gtest/gtest.h>

#include <cmath>
#include <random>
#include <vector>

#include "dsp/cfar.hpp"
#include "dsp/fft.hpp"
#include "dsp/spectrum.hpp"
#include "dsp/window.hpp"

namespace {

constexpr auto kPi = 3.14159265358979323846;

// Window, FFT and power spectrum of one block.
class Analyzer {
public:
  std::vector<float> power(const std::vector<float>& samples) const {
    auto buffer = std::vector<dsp::Complex>(dsp::kFftSize);
    window_.apply(samples.data(), buffer.data());
    fft_.forward(buffer.data());
    auto result = std::vector<float>(dsp::kSpectrumBins);
    dsp::powerSpectrum(buffer.data(), result.data());
    return result;
  }

private:
  dsp::HannWindow window_;
  dsp::Fft fft_;
};

std::vector<float> noisyTone(std::mt19937& engine, const std::size_t bin,
                             const float amplitude, const float sigma) {
  auto noise = std::normal_distribution<float>{0.0f, sigma};
  auto samples = std::vector<float>(dsp::kFftSize);
  for (auto n = std::size_t{0}; n < dsp::kFftSize; ++n) {
    const auto phase = 2.0 * kPi * bin * n / dsp::kFftSize;
    samples[n] =
        (amplitude * static_cast<float>(std::cos(phase))) + noise(engine);
  }
  return samples;
}

} // namespace

TEST(CaCfar, ThresholdOnFlatNoiseFollowsTheAlphaFormula) {
  constexpr auto kPfa = 1e-3f;
  const dsp::CaCfar cfar{kPfa};
  const auto power = std::vector<float>(dsp::kSpectrumBins, 2.0f);

  constexpr auto kCells = 2 * dsp::kTrainingCells;
  const auto alpha = kCells * (std::pow(kPfa, -1.0f / kCells) - 1.0f);

  EXPECT_NEAR(cfar.threshold(power.data(), 50), alpha * 2.0f, 1e-3f);
}

TEST(CaCfar, BinsAtTheEdgeUseTheTrainingCellsOnOneSide) {
  constexpr auto kPfa = 1e-3f;
  const dsp::CaCfar cfar{kPfa};
  const auto power = std::vector<float>(dsp::kSpectrumBins, 2.0f);

  constexpr auto kCells = dsp::kTrainingCells;
  const auto alpha = kCells * (std::pow(kPfa, -1.0f / kCells) - 1.0f);

  EXPECT_NEAR(cfar.threshold(power.data(), 0), alpha * 2.0f, 1e-3f);
  EXPECT_NEAR(cfar.threshold(power.data(), dsp::kSpectrumBins - 1),
              alpha * 2.0f, 1e-3f);
}

TEST(CaCfar, TrainingCellsExcludeTheGuardCells) {
  const dsp::CaCfar cfar{1e-3f};
  auto power = std::vector<float>(dsp::kSpectrumBins, 1.0f);
  const auto quiet = cfar.threshold(power.data(), 50);

  // A strong neighbour inside the guard cells must not raise the threshold.
  power[50 + dsp::kGuardCells] = 1000.0f;
  power[50 - dsp::kGuardCells] = 1000.0f;

  EXPECT_FLOAT_EQ(cfar.threshold(power.data(), 50), quiet);
}

TEST(CaCfar, FindsAToneInItsOwnBinAndNowhereElse) {
  const Analyzer analyzer;
  const dsp::CaCfar cfar{1e-6f};
  auto engine = std::mt19937{3};

  const auto power = analyzer.power(noisyTone(engine, 40, 1.0f, 0.05f));
  dsp::Detection found[dsp::kMaxDetections];
  const auto count = cfar.detect(power.data(), found, dsp::kMaxDetections);

  ASSERT_EQ(count, 1u);
  EXPECT_EQ(found[0].bin, 40);
  EXPECT_GT(found[0].power, found[0].threshold);
}

TEST(CaCfar, ThresholdFollowsTheNoiseLevel) {
  const Analyzer analyzer;
  const dsp::CaCfar cfar{1e-6f};
  auto engine = std::mt19937{4};

  const auto quietPower = analyzer.power(noisyTone(engine, 40, 1.0f, 0.05f));
  const auto loudPower = analyzer.power(noisyTone(engine, 40, 1.0f, 0.5f));

  dsp::Detection quiet[dsp::kMaxDetections];
  dsp::Detection loud[dsp::kMaxDetections];
  ASSERT_EQ(cfar.detect(quietPower.data(), quiet, dsp::kMaxDetections), 1u);
  ASSERT_EQ(cfar.detect(loudPower.data(), loud, dsp::kMaxDetections), 1u);

  // Noise 10x larger in amplitude is 100x larger in power.
  EXPECT_GT(loud[0].threshold / quiet[0].threshold, 50.0f);
  EXPECT_LT(loud[0].threshold / quiet[0].threshold, 200.0f);
}

TEST(CaCfar, FalseAlarmRateIsCloseToTheDesignedPfa) {
  constexpr auto kPfa = 1e-2f;
  constexpr auto kTrials = 4000;
  const Analyzer analyzer;
  const dsp::CaCfar cfar{kPfa};
  auto engine = std::mt19937{5};
  auto noise = std::normal_distribution<float>{0.0f, 1.0f};

  // Only bins with training cells on both sides.
  constexpr auto kFirst = dsp::kGuardCells + dsp::kTrainingCells;
  constexpr auto kLast = dsp::kSpectrumBins - 1 - kFirst;

  auto cells = 0.0;
  auto alarms = 0.0;
  for (auto trial = 0; trial < kTrials; ++trial) {
    auto samples = std::vector<float>(dsp::kFftSize);
    for (auto& sample : samples) {
      sample = noise(engine);
    }
    const auto power = analyzer.power(samples);
    for (auto bin = kFirst; bin <= kLast; ++bin) {
      cells += 1.0;
      if (power[bin] > cfar.threshold(power.data(), bin)) {
        alarms += 1.0;
      }
    }
  }

  const auto rate = alarms / cells;
  RecordProperty("measured_pfa", std::to_string(rate));
  EXPECT_GT(rate, kPfa * 0.5);
  EXPECT_LT(rate, kPfa * 2.0);
}
