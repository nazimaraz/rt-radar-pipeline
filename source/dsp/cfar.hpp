#pragma once

#include <cstddef>
#include <cstdint>

#include "dsp/fft.hpp"

namespace dsp {

constexpr auto kGuardCells{std::size_t{3}}; // each side of the cell under test
constexpr auto kTrainingCells{std::size_t{8}}; // each side
constexpr auto kMaxDetections{std::size_t{8}};

struct Detection {
  std::uint16_t bin;
  float power;
  float threshold;
};

// Trivially copyable, so it can travel through an OS queue.
struct DetectionList {
  std::uint32_t sequence;
  std::uint32_t count;
  Detection items[kMaxDetections];
};

// Cell-averaging CFAR on a power spectrum. The threshold for each bin is
// alpha * (mean power of the training cells around it), so it follows the
// noise floor and the false alarm probability stays constant.
//
// For exponentially distributed noise power and n independent training cells,
// Pfa = (1 + alpha / n)^-n, hence alpha = n * (Pfa^(-1/n) - 1).
class CaCfar {
public:
  explicit CaCfar(float falseAlarmProbability);

  // Threshold for bin `index` of a kSpectrumBins-long spectrum. Near the edges
  // there are training cells on one side only; with fewer than kTrainingCells
  // the bin cannot be tested and the threshold is infinite.
  [[nodiscard]] float threshold(const float* power, std::size_t index) const;

  // Writes the bins that exceed their threshold and are local maxima (a tone
  // spreads over a few bins; it is reported once). Lowest bins first, at most
  // `capacity`. Returns how many were written.
  [[nodiscard]] std::uint32_t detect(const float* power, Detection* out,
                                     std::size_t capacity) const;

private:
  float alpha_[(2 * kTrainingCells) + 1]{}; // indexed by training cell count
};

} // namespace dsp
