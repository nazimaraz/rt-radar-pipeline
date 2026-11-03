#include "dsp/fft.hpp"

#include <cmath>
#include <utility>

using namespace dsp;

namespace {

constexpr auto kTwoPi{6.28318530717958647692f};

static_assert((kFftSize & (kFftSize - 1)) == 0, "radix-2 needs a power of two");

[[nodiscard]] std::uint16_t reverseBits(const std::uint16_t value,
                                        const unsigned bits) {
  auto reversed{std::uint16_t{0}};
  for (auto bit{0u}; bit < bits; ++bit) {
    reversed = static_cast<std::uint16_t>(reversed << 1u);
    reversed |=
        static_cast<std::uint16_t>((static_cast<unsigned>(value) >> bit) & 1u);
  }
  return reversed;
}

} // namespace

Fft::Fft() {
  constexpr auto kSize{static_cast<float>(kFftSize)};
  for (auto k{std::size_t{0}}; k < kFftSize / 2; ++k) {
    const auto angle{(kTwoPi * static_cast<float>(k)) / kSize};
    cos_[k] = std::cos(angle);
    sin_[k] = std::sin(angle);
  }

  auto bits{0u};
  while ((std::size_t{1} << bits) < kFftSize) {
    ++bits;
  }

  for (auto i{std::size_t{0}}; i < kFftSize; ++i) {
    bitReverse_[i] = reverseBits(static_cast<std::uint16_t>(i), bits);
  }
}

void Fft::forward(Complex* const data) const {
  for (auto i{std::size_t{0}}; i < kFftSize; ++i) {
    const auto j{std::size_t{bitReverse_[i]}};
    if (i < j) {
      std::swap(data[i], data[j]);
    }
  }

  // W = exp(-j 2 pi k / N) = cos - j sin.
  for (auto length{std::size_t{2}}; length <= kFftSize; length <<= 1u) {
    const auto half{length / 2};
    const auto stride{kFftSize / length};
    for (auto start{std::size_t{0}}; start < kFftSize; start += length) {
      for (auto k{std::size_t{0}}; k < half; ++k) {
        const auto wr{cos_[k * stride]};
        const auto wi{-sin_[k * stride]};
        const auto top{data[start + k]};
        const auto bottom{data[start + k + half]};
        const auto tr{(bottom.re * wr) - (bottom.im * wi)};
        const auto ti{(bottom.re * wi) + (bottom.im * wr)};
        data[start + k] = Complex{top.re + tr, top.im + ti};
        data[start + k + half] = Complex{top.re - tr, top.im - ti};
      }
    }
  }
}
