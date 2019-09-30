#pragma once
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

namespace VolCorrelation {

#ifndef VolCorrelation_Vec3
#define VolCorrelation_Vec3
template <typename T> struct Vec3 {
  T x, y, z;
  Vec3() : x(0), y(0), z(0) {}
  Vec3(T a, T b, T c) : x(a), y(b), z(c) {}
  auto norm() const -> T { return std::sqrt(x * x + y * y + z * z); }
};
#endif

inline auto suma(std::vector<double> a) -> double {
  auto s = 0.0;
  for (auto i : a) {
    s += i;
  }
  return s;
}

inline auto mean(std::vector<double> &a) -> double {
  return suma(a) / a.size();
}

inline auto sqsum(std::vector<double> &a) -> double {
  auto s = 0.0;
  for (auto i : a) {
    s = s + pow(i, 2);
  }
  return s;
}

inline auto stdev(std::vector<double> &nums) -> double {
  auto N = static_cast<double>(nums.size());
  return pow(sqsum(nums) / N - pow(suma(nums) / N, 2), 0.5);
}

inline auto operator-(std::vector<double> &a, double b) -> std::vector<double> {
  std::vector<double> result;
  for (double i : a) {
    result.push_back(i - b);
  }
  return result;
}

inline auto operator*(std::vector<double> a, std::vector<double> b)
    -> std::vector<double> {
  std::vector<double> result;
  for (int i = 0; i < a.size(); i++) {
    result.push_back(a[i] * b[i]);
  }
  return result;
}

inline auto pearsoncoeff(std::vector<double> X, std::vector<double> Y)
    -> double {
  return suma((X - mean(X)) * (Y - mean(Y))) / (X.size() * stdev(X) * stdev(Y));
}

template <typename ResultType>
inline auto getLLC(const std::vector<ResultType> &fieldA,
                   const std::vector<ResultType> &fieldB,
                   const Vec3<uint32_t> &pos, const Vec3<uint32_t> &dimensions,
                   int offsetXY, int windowSize) -> ResultType {
  const auto num = static_cast<size_t>(pow(2 * windowSize - 1, 3));
  std::vector<ResultType> X;
  std::vector<ResultType> Y;
  X.reserve(num);
  Y.reserve(num);

  const auto px = static_cast<int>(pos.x);
  const auto py = static_cast<int>(pos.y);
  const auto pz = static_cast<int>(pos.z);

  const auto maxX = static_cast<int>(pos.x);
  const auto maxY = static_cast<int>(pos.y);
  const auto maxZ = static_cast<int>(pos.z);

  for (auto z = pz - windowSize; z <= pz + windowSize; z++) {
    if (z < 0 || z >= maxZ) {
      continue;
    }
    for (auto y = py - windowSize; y <= py + windowSize; y++) {
      if (y < 0 || y >= maxY) {
        continue;
      }
      for (auto x = px - windowSize; x <= px + windowSize; x++) {
        if (x < 0 || x >= maxX) {
          continue;
        }
        auto idx = z * offsetXY + y * maxX + x;
        X.push_back(fieldA[idx]);
        Y.push_back(fieldB[idx]);
      }
    }
  }

  const auto sumaX = suma(X);
  const auto sumaY = suma(Y);
  const auto meanX = sumaX / X.size();
  const auto meanY = sumaY / Y.size();
  const auto sqsumX = sqsum(X);
  const auto sqsumY = sqsum(Y);
  const auto stdevX = sqrt(sqsumX / X.size() - pow(sumaX / X.size(), 2));
  const auto stdevY = sqrt(sqsumY / Y.size() - pow(sumaY / Y.size(), 2));

  // auto p = pearsoncoeff(X, Y);
  auto p = suma((X - meanX) * (Y - meanY)) / (X.size() * stdevX * stdevY);
  if (-1 < p && p < 1) {
    return static_cast<ResultType>(abs(p));
  }

  return 0;
}

template <typename T, typename ResultType = float>
auto calcLocalCorrelationCoefficient(const std::vector<T *> &fields,
                                     uint32_t width, uint32_t height,
                                     uint32_t depth, int windowSize = 3)
    -> std::vector<ResultType> {
  using std::vector;

  auto dimensions = Vec3<uint32_t>(width, height, depth);
  auto offsetZ = width * height;
  auto size = depth * offsetZ;

  // normalize fields
  vector<vector<ResultType>> normalizeds;
  for (auto field : fields) {
    vector<ResultType> normalized(size);

    T max = field[0];
    for (size_t idx = 0; idx < size; idx++) {
      const auto value = field[idx];
      if (value > max) {
        max = value;
      }
    }
    for (size_t idx = 0; idx < size; idx++) {
      normalized[idx] = static_cast<ResultType>(field[idx]) / max;
    }
    normalizeds.push_back(std::move(normalized));
  }

  auto total = dimensions.x * dimensions.y * dimensions.z;
  auto offsetXY = dimensions.x * dimensions.y;

  auto result = vector<ResultType>(total, 1.0);

  for (auto i = 0; i < fields.size(); i++) {
    for (auto j = i + 1; j < fields.size(); j++) {
      auto pos = Vec3<uint32_t>();
      auto index = 0ULL;
      for (pos.z = 0; pos.z < depth; pos.z++) {
        for (pos.y = 0; pos.y < height; pos.y++) {
          for (pos.x = 0; pos.x < width; pos.x++) {
            const auto value = getLLC(normalizeds[i], normalizeds[j], pos,
                                      dimensions, offsetXY, windowSize);
            const auto exist = result[index];
            result[index] = exist < value ? exist : value;
            index++;
          }
        }
      }
    }
  }

  return result;
}

} // namespace VolCorrelation
