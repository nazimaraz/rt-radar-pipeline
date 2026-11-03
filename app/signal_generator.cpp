#include "signal_generator.hpp"

namespace sim {
namespace {

constexpr auto kSweepBlocks{std::uint32_t{100}};
constexpr auto kTargetABaseHz{1'500.f};
constexpr auto kTargetAStepHz{10.f};
constexpr auto kTargetBBaseHz{5'000.f};
constexpr auto kTargetBStepHz{-8.f};
constexpr auto kAmplitudeA{1.f};
constexpr auto kAmplitudeB{0.5f};
constexpr auto kNoiseAmplitude{0.2f};
constexpr auto kInt32ToUnit{1.f / 2'147'483'648.f};

[[nodiscard]] float absolute(const float value) {
  return value < 0.f ? -value : value;
}

[[nodiscard]] float sineTurns(const float turns) {
  const auto centered{turns >= 0.5f ? turns - 1.f : turns};
  const auto parabola{(8.f * centered) -
                      (16.f * centered * absolute(centered))};
  return parabola + (0.225f * ((parabola * absolute(parabola)) - parabola));
}

[[nodiscard]] float advance(const float phase, const float step) {
  const auto next{phase + step};
  return next >= 1.0f ? next - 1.0f : next;
}

} // namespace

float SignalGenerator::nextNoise() {
  rngState_ ^= rngState_ << 13u;
  rngState_ ^= rngState_ >> 17u;
  rngState_ ^= rngState_ << 5u;
  return static_cast<float>(static_cast<std::int32_t>(rngState_)) *
         kInt32ToUnit;
}

void SignalGenerator::fill(float* const samples, const std::size_t count,
                           const std::uint32_t blockIndex) {
  const auto sweep{static_cast<float>(blockIndex % kSweepBlocks)};
  constexpr auto kRate{static_cast<float>(kSampleRateHz)};
  const auto stepA{(kTargetABaseHz + (kTargetAStepHz * sweep)) / kRate};
  const auto stepB{(kTargetBBaseHz + (kTargetBStepHz * sweep)) / kRate};
  for (auto i{std::size_t{0}}; i < count; ++i) {
    samples[i] = (kAmplitudeA * sineTurns(phaseA_)) +
                 (kAmplitudeB * sineTurns(phaseB_)) +
                 (kNoiseAmplitude * nextNoise());
    phaseA_ = advance(phaseA_, stepA);
    phaseB_ = advance(phaseB_, stepB);
  }
}

} // namespace sim
