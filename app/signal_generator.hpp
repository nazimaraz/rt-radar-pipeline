#pragma once

#include <cstddef>
#include <cstdint>

namespace sim {

constexpr auto kSampleRateHz{std::uint32_t{16'000}};

// Simulated ADC: noise plus two tones whose frequencies sweep, delivered as
// raw big-endian words (dsp/adc.hpp), the way the hardware would.
class SignalGenerator {
public:
  void fill(std::uint16_t* raw, std::size_t count, std::uint32_t blockIndex);

private:
  [[nodiscard]] float nextNoise();

  std::uint32_t rngState_{0x2545F491u};
  float phaseA_{0.f};
  float phaseB_{0.f};
};

} // namespace sim
