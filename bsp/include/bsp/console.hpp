#pragma once

#include <cstdint>
#include <type_traits>

// Heap-free console on UART0.
//
// Not thread-safe on purpose: two tasks printing at once interleave their
// characters. All output is meant to go through a logger task instead of
// every task calling this directly.
namespace bsp::console {

void putChar(char c);
void write(const char* text);
void writeUnsigned(std::uint32_t value, unsigned base = 10);
void writeSigned(std::int32_t value);
void writeFloat(float value);  // fixed 3 decimals, no dtoa and no heap

template <typename T>
void printOne(const T& value) {
    if constexpr (std::is_same_v<T, bool>) {
        write(value ? "true" : "false");
    } else if constexpr (std::is_same_v<T, char>) {
        putChar(value);
    } else if constexpr (std::is_floating_point_v<T>) {
        writeFloat(static_cast<float>(value));
    } else if constexpr (std::is_integral_v<T>) {
        static_assert(sizeof(T) <= 4, "64-bit integers are not supported");
        if constexpr (std::is_signed_v<T>) {
            writeSigned(static_cast<std::int32_t>(value));
        } else {
            writeUnsigned(static_cast<std::uint32_t>(value));
        }
    } else {
        write(value);  // C string
    }
}

// print("beat ", n, " at ", ms, " ms\n"): type-safe, no format string.
template <typename... Args>
void print(const Args&... args) {
    (printOne(args), ...);
}

}  // namespace bsp::console
