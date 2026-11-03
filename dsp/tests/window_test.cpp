#include <gtest/gtest.h>

#include <vector>

#include "dsp/window.hpp"

TEST(HannWindow, IsZeroAtTheStartAndOneInTheMiddle) {
  const dsp::HannWindow window;

  EXPECT_FLOAT_EQ(window.coefficient(0), 0.0f);
  EXPECT_NEAR(window.coefficient(dsp::kFftSize / 2), 1.0f, 1e-6f);
}

TEST(HannWindow, IsSymmetricAroundTheMiddle) {
  const dsp::HannWindow window;

  for (auto n = std::size_t{1}; n < dsp::kFftSize; ++n) {
    EXPECT_NEAR(window.coefficient(n), window.coefficient(dsp::kFftSize - n),
                1e-6f);
  }
}

TEST(HannWindow, CoherentGainIsOneHalf) {
  const dsp::HannWindow window;

  auto sum = 0.0;
  for (auto n = std::size_t{0}; n < dsp::kFftSize; ++n) {
    sum += window.coefficient(n);
  }
  EXPECT_NEAR(sum / dsp::kFftSize, 0.5, 1e-6);
}

TEST(HannWindow, ApplyScalesSamplesAndClearsTheImaginaryPart) {
  const dsp::HannWindow window;
  const auto samples = std::vector<float>(dsp::kFftSize, 2.0f);
  auto out = std::vector<dsp::Complex>(dsp::kFftSize, dsp::Complex{9.0f, 9.0f});

  window.apply(samples.data(), out.data());

  for (auto n = std::size_t{0}; n < dsp::kFftSize; ++n) {
    EXPECT_FLOAT_EQ(out[n].re, 2.0f * window.coefficient(n));
    EXPECT_FLOAT_EQ(out[n].im, 0.0f);
  }
}
