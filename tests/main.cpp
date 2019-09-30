#include "VolCorrelation/GradientSimilarityMeasure.hpp"
#include <iostream>
#include <vector>

using namespace std;
using namespace VolCorrelation;

auto main() -> int {
  vector<float> A(100, 1.0f);
  vector<float> B(100, 0.0f);

  vector<float *> fields = {A.data(), B.data()};
  const auto res = calculateGradientSimilarity(fields, 100, 1, 1);
  for (auto v : res) {
    cout << v << " ";
  }
  cout << endl;

  return 0;
}
