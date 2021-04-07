#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

namespace Info {

using Counts = std::vector<size_t>;

constexpr size_t BucketNum = 256 - 1;

// convert data to type of uint8_t
template <typename T>
std::vector<uint8_t> ConvertData(const std::vector<T> &data) {
  std::vector<uint8_t> result;
  result.reserve(data.size());

  T max = data[0];
  T min = data[0];
  std::for_each(data.begin(), data.end(), [&max, &min](auto value) {
    if (value > max) {
      max = value;
    } else if (value < min) {
      min = value;
    }
  });

  double width = static_cast<double>(max) - static_cast<double>(min);
  std::for_each(
      data.begin(), data.end(), [&result, width, min, max](auto value) {
        auto w = static_cast<double>(value) - static_cast<double>(min);
        auto newValue = w / width * BucketNum;
        result.push_back(static_cast<uint8_t>(newValue));
      });

  return result;
}

inline Counts CountValue(const std::vector<uint8_t> &data) {
  Counts counts(BucketNum + 1, 0);

  for (auto value : data) {
    counts[value] += 1;
  }

  // remove noise
  for (size_t i = 0; i < counts.size(); i++) {
    if (counts[i] <= 1) {
      counts[0] += counts[i];
      counts[i] = 0;
    }
  }

  return counts;
}

inline double CalculateEntropy(Counts &counts, size_t total) {
  auto entropy = 0.0;
  for (auto count : counts) {
    if (count == 0) continue;
    auto prob = static_cast<double>(count) / total;
    entropy += prob * log(prob);
  }

  return -entropy;
}

inline double CalculateMutationInformation(uint8_t *fieldA, uint8_t *fieldB, Counts &a,
                                    Counts &b, size_t size) {
  assert(size != 0);

  auto mi = 0.0;

  std::vector<std::vector<size_t>> counts(BucketNum + 1);
  for (auto &c : counts) {
    c.resize(BucketNum + 1, 0);
  }

  for (size_t i = 0; i < size; i++) {
    counts[fieldA[i]][fieldB[i]] += 1;
  }

  for (size_t i = 0; i < BucketNum + 1; i++) {
    for (size_t j = 0; j < BucketNum + 1; j++) {
      if (counts[i][j] == 0 || a[i] == 0 || b[j] == 0) {
        continue;
      }
      auto count = static_cast<double>(counts[i][j]);
      auto pmi = count / size * log(count * size / (a[i] * b[j]));
      mi += pmi;
    }
  }

  return mi;
}
} // namespace Info
