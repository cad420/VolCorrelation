#include "GradientSimilarityMeasure.hpp"
#include <iostream>

using namespace std;

auto main() -> int {
  vector<float> A(100, 1.0f);
  vector<float> B(100, 0.0f);

  vector<float *> fields = { A.data(), B.data() };
  const auto res = GSIM::calculateSimilarity(fields, 100, 1, 1);
  for (auto v : res) {
    cout << v << " ";
  }
  cout << endl;

  return 0;
}