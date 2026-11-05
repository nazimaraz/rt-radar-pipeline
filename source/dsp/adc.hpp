#pragma once

#include <cstddef>
#include <cstdint>

namespace dsp {

// Sample format of the ADC: 16-bit two's complement words in big-endian
// (network) byte order, as a VITA 49 payload carries them. A code of 32767
// is just under +kAdcFullScale, -32768 is -kAdcFullScale.
constexpr auto kAdcFullScale{2.0f};

// The raw word the ADC stores for `value`, clipped to full scale.
[[nodiscard]] std::uint16_t encodeSample(float value);

// Converts `count` raw words to floats in [-kAdcFullScale, kAdcFullScale).
void decodeSamples(const std::uint16_t* raw, float* out, std::size_t count);

} // namespace dsp
