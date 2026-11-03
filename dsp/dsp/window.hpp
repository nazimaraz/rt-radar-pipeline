#pragma once

#include <cstddef>

#include "dsp/fft.hpp"

namespace dsp {

// Periodic Hann window, w[n] = 0.5 * (1 - cos(2 pi n / N)). It tapers the block
// to zero at both ends, so a tone that does not fit a whole number of cycles in
// the block leaks into far fewer bins than with no window.
class HannWindow {
public:
  HannWindow();

  // Stores samples[n] * w[n] as the real part of out[n]; the imaginary part is
  // cleared. Both arrays hold kFftSize elements.
  void apply(const float* samples, Complex* out) const;

  [[nodiscard]] float coefficient(std::size_t index) const;

private:
  float coefficients_[kFftSize]{};
};

} // namespace dsp
