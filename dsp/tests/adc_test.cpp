#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <cstring>

#include "dsp/adc.hpp"

namespace {

constexpr auto kLsb = dsp::kAdcFullScale / 32768.0f;

// A raw word's bytes in memory order, whatever the host's byte order.
struct Bytes {
  std::uint8_t first;
  std::uint8_t second;
};

Bytes bytesOf(const std::uint16_t word) {
  Bytes bytes{};
  std::memcpy(&bytes, &word, sizeof(word));
  return bytes;
}

std::uint16_t wordFrom(const std::uint8_t first, const std::uint8_t second) {
  const std::uint8_t bytes[2] = {first, second};
  std::uint16_t word = 0;
  std::memcpy(&word, bytes, sizeof(word));
  return word;
}

float decodeOne(const std::uint16_t word) {
  float value = 0.0f;
  dsp::decodeSamples(&word, &value, 1);
  return value;
}

} // namespace

TEST(Adc, EncodesMostSignificantByteFirst) {
  const auto bytes = bytesOf(dsp::encodeSample(0x1234 * kLsb));
  EXPECT_EQ(bytes.first, 0x12);
  EXPECT_EQ(bytes.second, 0x34);
}

TEST(Adc, DecodesMostSignificantByteFirst) {
  EXPECT_EQ(decodeOne(wordFrom(0x12, 0x34)), 0x1234 * kLsb);
  EXPECT_EQ(decodeOne(wordFrom(0xFF, 0xFE)), -2.0f * kLsb); // two's complement
}

TEST(Adc, ClipsAtFullScale) {
  const auto high = bytesOf(dsp::encodeSample(10.0f));
  EXPECT_EQ(high.first, 0x7F);
  EXPECT_EQ(high.second, 0xFF);

  const auto low = dsp::encodeSample(-10.0f);
  EXPECT_EQ(bytesOf(low).first, 0x80);
  EXPECT_EQ(bytesOf(low).second, 0x00);
  EXPECT_EQ(decodeOne(low), -dsp::kAdcFullScale);
}

TEST(Adc, RoundTripsWithinOneCode) {
  for (auto i = -190; i <= 190; ++i) {
    const auto value = static_cast<float>(i) / 100.0f;
    EXPECT_LT(std::abs(decodeOne(dsp::encodeSample(value)) - value), kLsb)
        << "value " << value;
  }
}
