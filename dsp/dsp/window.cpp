#include "dsp/window.hpp"

#include <cmath>

namespace dsp {
namespace {

constexpr auto kTwoPi{6.28318530717958647692f};

} // namespace

HannWindow::HannWindow() {
  constexpr auto kSize{static_cast<float>(kFftSize)};
  for (auto n{std::size_t{0}}; n < kFftSize; ++n) {
    const auto phase{(kTwoPi * static_cast<float>(n)) / kSize};
    coefficients_[n] = 0.5f * (1.0f - std::cos(phase));
  }
}

void HannWindow::apply(const float* const samples, Complex* const out) const {
  for (auto n{std::size_t{0}}; n < kFftSize; ++n) {
    out[n] = Complex{.re = samples[n] * coefficients_[n], .im = 0.0f};
  }
}

float HannWindow::coefficient(const std::size_t index) const {
  return coefficients_[index];
}

} // namespace dsp
