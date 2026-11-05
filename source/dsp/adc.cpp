#include "dsp/adc.hpp"

#include <algorithm>
#include <bit>

namespace dsp {
namespace {

static_assert(std::endian::native == std::endian::little ||
                  std::endian::native == std::endian::big,
              "mixed-endian targets are not supported");

constexpr auto kCodesPerUnit{32'768.0f / kAdcFullScale};

// Swaps between big-endian and native order; the same swap goes both ways.
[[nodiscard]] constexpr std::uint16_t bigEndian(const std::uint16_t word) {
  if constexpr (std::endian::native == std::endian::little) {
    return std::byteswap(word);
  } else {
    return word;
  }
}

} // namespace

std::uint16_t encodeSample(const float value) {
  const auto clipped{std::clamp(value * kCodesPerUnit, -32'768.0f, 32'767.0f)};
  const auto code{static_cast<std::int16_t>(clipped)};
  return bigEndian(std::bit_cast<std::uint16_t>(code));
}

void decodeSamples(const std::uint16_t* const raw, float* const out,
                   const std::size_t count) {
  for (auto i{std::size_t{0}}; i < count; ++i) {
    const auto code{std::bit_cast<std::int16_t>(bigEndian(raw[i]))};
    out[i] = static_cast<float>(code) / kCodesPerUnit;
  }
}

} // namespace dsp
