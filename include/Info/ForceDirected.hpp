#pragma once

#include "HierarchicalCluster.hpp"
#include <array>
#include <iostream>
#include <random>
#include <utility>

namespace Info {
inline std::array<double, 2>
CalculatePartialDerivatives(const std::vector<std::array<double, 2>> &particles,
                            const std::vector<std::vector<double>> &l,
                            const std::vector<std::vector<double>> &k,
                            size_t idx) {
  double resX = 0.0;
  double resY = 0.0;
  const auto &current = particles[idx];
  for (size_t i = 0; i < particles.size(); i++) {
    if (i == idx) {
      continue;
    }
    auto x_m = current[0];
    auto x_i = particles[i][0];
    auto y_m = current[1];
    auto y_i = particles[i][1];
    auto k_mi = k[idx][i];
    auto l_mi = l[idx][i];
    auto delta_x = x_m - x_i;
    auto delta_x_squr = delta_x * delta_x;
    auto delta_y = y_m - y_i;
    auto delta_y_squr = delta_y * delta_y;
    auto distance = sqrt(delta_x_squr + delta_y_squr);
    resX += k_mi * (delta_x - (l_mi * delta_x / distance));
    resY += k_mi * (delta_y - (l_mi * delta_y / distance));
  }
  return std::array<double, 2>{resX, resY};
}

inline std::array<double, 4> CalculateSecondPartialDerivatives(
    const std::vector<std::array<double, 2>> &particles,
    const std::vector<std::vector<double>> &l,
    const std::vector<std::vector<double>> &k, size_t idx) {
  double resX = 0.0;
  double resY = 0.0;
  double resXY = 0.0;
  const auto &current = particles[idx];
  for (size_t i = 0; i < particles.size(); i++) {
    if (i == idx) {
      continue;
    }
    auto x_m = current[0];
    auto x_i = particles[i][0];
    auto y_m = current[1];
    auto y_i = particles[i][1];
    auto k_mi = k[idx][i];
    auto l_mi = l[idx][i];
    auto delta_x = x_m - x_i;
    auto delta_x_squr = delta_x * delta_x;
    auto delta_y = y_m - y_i;
    auto delta_y_squr = delta_y * delta_y;
    auto dividor = pow(delta_x_squr + delta_y_squr, 1.5);
    resX += k_mi * (1 - (l_mi * delta_y_squr / dividor));
    resY += k_mi * (1 - (l_mi * delta_x_squr / dividor));
    resXY += k_mi * l_mi * delta_x * delta_y / dividor;
  }
  return std::array<double, 4>{resX, resXY, resXY, resY};
}

inline double CalculateEnergeDelta(const std::vector<std::array<double, 2>> &particles,
                            const std::vector<std::vector<double>> &l,
                            const std::vector<std::vector<double>> &k,
                            size_t idx) {
  auto res = CalculatePartialDerivatives(particles, l, k, idx);
  return sqrt(res[0] * res[0] + res[1] * res[1]);
}

inline std::array<double, 2>
CalculateMove(const std::vector<std::array<double, 2>> &particles,
              const std::vector<std::vector<double>> &l,
              const std::vector<std::vector<double>> &k, size_t idx) {
  auto coef = CalculateSecondPartialDerivatives(particles, l, k, idx);
  auto rhs = CalculatePartialDerivatives(particles, l, k, idx);

  auto X = (-rhs[0] * coef[3] - -rhs[1] * coef[1]) /
           (coef[0] * coef[3] - coef[2] * coef[1]);
  auto Y = (-rhs[0] * coef[2] - -rhs[1] * coef[0]) /
           (coef[1] * coef[2] - coef[3] * coef[0]);
  return std::array<double, 2>{X, Y};
}

inline std::vector<std::array<double, 2>>
CalculateForceDirected(const std::vector<std::vector<double>> &ds,
                       uint32_t width, uint32_t height) {
  const auto num = ds.size() + 1;

  // add a fake particle at center of the graph
  // to let the layout keep in screen
  auto distances = ds;
  for (auto &d : distances) {
    d.push_back(30);
  }

  std::vector<std::array<double, 2>> particles(num);

  if (ds.size() < 2) {
    particles.pop_back();
    return particles;
  }

  // randomly initialize p1, p2, ..., pn;
  std::mt19937 rng;
  rng.seed(12345);
  std::uniform_int_distribution<uint32_t> distrib(50, width - 50);
  for (auto &p : particles) {
    p[0] = distrib(rng);
    p[1] = distrib(rng);
  }
  particles[num - 1][0] = width / 2;
  particles[num - 1][1] = height / 2;

  // compute l_ij for 1 <= i != j <= n
  const auto L0 = width;
  auto L = distances[0][1];
  for (size_t i = 0; i < num; i++) {
    for (size_t j = i + 1; j < num; j++) {
      if (distances[i][j] > L) {
        L = distances[i][j];
      }
    }
  }
  L = L0 / L;
  std::vector<std::vector<double>> l(num);
  for (size_t i = 0; i < num; i++) {
    l[i].resize(num);
    for (size_t j = i + 1; j < num; j++) {
      l[i][j] = L * distances[i][j];
    }
  }

  // compute k_ij for 1 <= i != j <= n
  const auto K = 1.0;
  std::vector<std::vector<double>> k(num);
  for (size_t i = 0; i < num; i++) {
    k[i].resize(num);
    for (size_t j = i + 1; j < num; j++) {
      k[i][j] = K / (distances[i][j] * distances[i][j]);
    }
  }

  const double exp = 1e-8;
  double maxDelta = 0.0;
  size_t maxIdx = 0;

  for (size_t i = 0; i < particles.size(); i++) {
    auto tmp = CalculateEnergeDelta(particles, l, k, i);
    if (tmp > maxDelta) {
      maxDelta = tmp;
      maxIdx = i;
    }
  }

  size_t count = 0;
  while (maxDelta > exp) {
    // let p_m be the particle satisfying delta_m = max_i_delta_i
    auto delta = FLT_MAX;
    auto &p = particles[maxIdx];
    while (delta > exp) {
      auto move = CalculateMove(particles, l, k, maxIdx);
      p[0] += move[0];
      p[1] += move[1];
      delta = CalculateEnergeDelta(particles, l, k, maxIdx);
      count++;
    }

    maxDelta = 0.0;
    for (size_t i = 0; i < particles.size(); i++) {
      auto tmp = CalculateEnergeDelta(particles, l, k, i);
      if (tmp > maxDelta) {
        maxDelta = tmp;
        maxIdx = i;
      }
    }
  }

  std::cout << "Force-directed iteration: " << count << std::endl;

  particles.pop_back();
  return particles;
}

} // namespace Info
