#pragma once

#include <cstddef>
#include <cstdint>

namespace dsp {

constexpr auto kFftSize{std::size_t{256}};
constexpr auto kSpectrumBins{(kFftSize / 2) + 1};

struct Complex {
  float re;
  float im;
};

// Radix-2 decimation-in-time FFT of kFftSize points. The twiddle factors and
// the bit-reversal table are computed once, in the constructor.
class Fft {
public:
  Fft();

  // In place: `data` holds kFftSize samples on entry and the spectrum on exit.
  void forward(Complex* data) const;

private:
  float cos_[kFftSize / 2]{};
  float sin_[kFftSize / 2]{};
  std::uint16_t bitReverse_[kFftSize]{};
};

} // namespace dsp
