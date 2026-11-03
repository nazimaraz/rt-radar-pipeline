#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

// Heap-free, type-safe text formatting into any character sink.
//
// A sink is any type with `void put(char)`: the UART console and the logger's
// message buffer both use this code, so there is one formatter, not two.
namespace bsp::format {

template <typename Sink>
void write(Sink& sink, const char* text) {
  while (*text != '\0') {
    sink.put(*text++);
  }
}

template <typename Sink>
void writeUnsigned(Sink& sink, std::uint32_t value, unsigned base = 10) {
  if (base < 2u || base > 16u) {
    base = 10u;
  }
  char digits[32];
  auto count{std::size_t{0}};
  do {
    digits[count++] = "0123456789abcdef"[value % base];
    value /= base;
  } while (value != 0u);
  while (count > 0u) {
    sink.put(digits[--count]);
  }
}

template <typename Sink>
void writeSigned(Sink& sink, const std::int32_t value) {
  if (value < 0) {
    sink.put('-');
    writeUnsigned(sink, 0u - static_cast<std::uint32_t>(value));
  } else {
    writeUnsigned(sink, static_cast<std::uint32_t>(value));
  }
}

// Fixed 3 decimals, no dtoa and no heap.
template <typename Sink>
void writeFloat(Sink& sink, float value) {
  if (value != value) {
    write(sink, "nan");
    return;
  }
  if (value < 0.0f) {
    sink.put('-');
    value = -value;
  }
  if (value >= 4.0e9f) {
    write(sink, "inf");
    return;
  }
  // 32-bit only: float -> uint32 is one VCVT instruction, while a 64-bit
  // conversion would pull in software double-precision routines.
  auto whole{static_cast<std::uint32_t>(value)};
  // NOLINTBEGIN(bugprone-incorrect-roundings): lround would pull in libm
  const auto scaled{(value - static_cast<float>(whole)) * 1000.0f};
  auto fraction{static_cast<std::uint32_t>(scaled + 0.5f)};
  // NOLINTEND(bugprone-incorrect-roundings)
  if (fraction >= 1000u) {
    ++whole;
    fraction -= 1000u;
  }
  writeUnsigned(sink, whole);
  sink.put('.');
  if (fraction < 100u) {
    sink.put('0');
  }
  if (fraction < 10u) {
    sink.put('0');
  }
  writeUnsigned(sink, fraction);
}

template <typename Sink, typename T>
void printOne(Sink& sink, const T& value) {
  if constexpr (std::is_same_v<T, bool>) {
    write(sink, value ? "true" : "false");
  } else if constexpr (std::is_same_v<T, char>) {
    sink.put(value);
  } else if constexpr (std::is_floating_point_v<T>) {
    writeFloat(sink, static_cast<float>(value));
  } else if constexpr (std::is_integral_v<T>) {
    static_assert(sizeof(T) <= 4, "64-bit integers are not supported");
    if constexpr (std::is_signed_v<T>) {
      writeSigned(sink, static_cast<std::int32_t>(value));
    } else {
      writeUnsigned(sink, static_cast<std::uint32_t>(value));
    }
  } else {
    write(sink, value); // C string
  }
}

// print(sink, "beat ", n, " at ", ms, " ms\n"): no format string to get wrong.
template <typename Sink, typename... Args>
void print(Sink& sink, const Args&... args) {
  (printOne(sink, args), ...);
}

} // namespace bsp::format
