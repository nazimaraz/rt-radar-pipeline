#include "dsp/cfar.hpp"

#include <cmath>
#include <limits>

namespace dsp {

CaCfar::CaCfar(const float falseAlarmProbability) {
  for (auto n{std::size_t{1}}; n < (2 * kTrainingCells) + 1; ++n) {
    const auto cells{static_cast<float>(n)};
    alpha_[n] = cells * (std::pow(falseAlarmProbability, -1.0f / cells) - 1.0f);
  }
}

float CaCfar::threshold(const float* const power,
                        const std::size_t index) const {
  auto sum{0.0f};
  auto cells{std::size_t{0}};

  if (index > kGuardCells) {
    const auto last{index - kGuardCells - 1};
    const auto first{last >= kTrainingCells - 1 ? last - (kTrainingCells - 1)
                                                : std::size_t{0}};
    for (auto i{first}; i <= last; ++i) {
      sum += power[i];
      ++cells;
    }
  }

  const auto rightFirst{index + kGuardCells + 1};
  for (auto i{rightFirst}; i < rightFirst + kTrainingCells && i < kSpectrumBins;
       ++i) {
    sum += power[i];
    ++cells;
  }

  if (cells < kTrainingCells) {
    return std::numeric_limits<float>::infinity();
  }
  return alpha_[cells] * (sum / static_cast<float>(cells));
}

std::uint32_t CaCfar::detect(const float* const power, Detection* const out,
                             const std::size_t capacity) const {
  auto written{std::uint32_t{0}};
  for (auto bin{std::size_t{0}}; bin < kSpectrumBins && written < capacity;
       ++bin) {
    const auto left{bin > 0 ? power[bin - 1] : 0.0f};
    const auto right{bin + 1 < kSpectrumBins ? power[bin + 1] : 0.0f};
    if (power[bin] < left || power[bin] <= right) {
      continue;
    }

    const auto limit{threshold(power, bin)};
    if (power[bin] > limit) {
      out[written] = Detection{.bin = static_cast<std::uint16_t>(bin),
                               .power = power[bin],
                               .threshold = limit};
      ++written;
    }
  }
  return written;
}

} // namespace dsp
