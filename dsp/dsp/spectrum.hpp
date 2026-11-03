#pragma once

#include "dsp/fft.hpp"

namespace dsp {

// |X[k]|^2 for the kSpectrumBins bins from DC to Nyquist. Power rather than
// magnitude: no square root, and noise power is exponentially distributed,
// which is what the CFAR threshold formula assumes.
void powerSpectrum(const Complex* spectrum, float* power);

} // namespace dsp
