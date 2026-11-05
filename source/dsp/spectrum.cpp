#include "dsp/spectrum.hpp"

namespace dsp {

void powerSpectrum(const Complex* const spectrum, float* const power) {
  for (auto k{std::size_t{0}}; k < kSpectrumBins; ++k) {
    power[k] =
        (spectrum[k].re * spectrum[k].re) + (spectrum[k].im * spectrum[k].im);
  }
}

} // namespace dsp
