#include <gtest/gtest.h>

#include <cmath>
#include <random>
#include <vector>

#include "dsp/fft.hpp"
#include "dsp/spectrum.hpp"

namespace {

constexpr auto kPi = 3.14159265358979323846;

const dsp::Fft& fft() {
  static const dsp::Fft instance;
  return instance;
}

std::vector<dsp::Complex> randomBlock(const unsigned seed) {
  auto engine = std::mt19937{seed};
  auto uniform = std::uniform_real_distribution<float>{-1.0f, 1.0f};
  auto block = std::vector<dsp::Complex>(dsp::kFftSize);
  for (auto& value : block) {
    value = dsp::Complex{uniform(engine), uniform(engine)};
  }
  return block;
}

} // namespace

TEST(Fft, ImpulseHasFlatSpectrum) {
  auto data =
      std::vector<dsp::Complex>(dsp::kFftSize, dsp::Complex{0.0f, 0.0f});
  data[0] = dsp::Complex{1.0f, 0.0f};

  fft().forward(data.data());

  for (const auto& bin : data) {
    EXPECT_NEAR(bin.re, 1.0f, 1e-6f);
    EXPECT_NEAR(bin.im, 0.0f, 1e-6f);
  }
}

TEST(Fft, CosineLandsInTwoBins) {
  constexpr auto kBin = std::size_t{17};
  auto data = std::vector<dsp::Complex>(dsp::kFftSize);
  for (auto n = std::size_t{0}; n < dsp::kFftSize; ++n) {
    const auto phase = 2.0 * kPi * kBin * n / dsp::kFftSize;
    data[n] = dsp::Complex{static_cast<float>(std::cos(phase)), 0.0f};
  }

  fft().forward(data.data());

  for (auto k = std::size_t{0}; k < dsp::kFftSize; ++k) {
    const auto magnitude = std::hypot(data[k].re, data[k].im);
    if (k == kBin || k == dsp::kFftSize - kBin) {
      EXPECT_NEAR(magnitude, dsp::kFftSize / 2.0, 1e-3) << "bin " << k;
    } else {
      EXPECT_NEAR(magnitude, 0.0, 1e-3) << "bin " << k;
    }
  }
}

TEST(Fft, MatchesNaiveDft) {
  const auto input = randomBlock(1);
  auto data = input;

  fft().forward(data.data());

  for (auto k = std::size_t{0}; k < dsp::kFftSize; ++k) {
    auto re = 0.0;
    auto im = 0.0;
    for (auto n = std::size_t{0}; n < dsp::kFftSize; ++n) {
      const auto angle = -2.0 * kPi * k * n / dsp::kFftSize;
      re += (input[n].re * std::cos(angle)) - (input[n].im * std::sin(angle));
      im += (input[n].re * std::sin(angle)) + (input[n].im * std::cos(angle));
    }
    EXPECT_NEAR(data[k].re, re, 2e-3) << "bin " << k;
    EXPECT_NEAR(data[k].im, im, 2e-3) << "bin " << k;
  }
}

TEST(Fft, ObeysParseval) {
  const auto input = randomBlock(2);
  auto data = input;

  fft().forward(data.data());

  auto timeEnergy = 0.0;
  auto frequencyEnergy = 0.0;
  for (auto n = std::size_t{0}; n < dsp::kFftSize; ++n) {
    timeEnergy += (input[n].re * input[n].re) + (input[n].im * input[n].im);
    frequencyEnergy += (data[n].re * data[n].re) + (data[n].im * data[n].im);
  }
  EXPECT_NEAR(frequencyEnergy / dsp::kFftSize, timeEnergy, timeEnergy * 1e-4);
}

TEST(PowerSpectrum, IsSquaredMagnitudeOfEachBin) {
  auto spectrum =
      std::vector<dsp::Complex>(dsp::kFftSize, dsp::Complex{3.0f, 4.0f});
  auto power = std::vector<float>(dsp::kSpectrumBins);

  dsp::powerSpectrum(spectrum.data(), power.data());

  for (const auto value : power) {
    EXPECT_FLOAT_EQ(value, 25.0f);
  }
}
