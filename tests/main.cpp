#include "VolCorrelation/GradientSimilarityMeasure.hpp"
#include "VolCorrelation/LocalCorrelationCoefficient.hpp"
#include <iostream>
#include <vector>

using namespace std;
using namespace VolCorrelation;

auto main() -> int {
  vector<double> A(100, 1.0);
  vector<double> B(100, 0.0);

  vector<double *> fields = {A.data(), B.data()};
  const auto res1 = calculateGradientSimilarity(fields, 100, 1, 1);
  for (auto v : res1) {
    cout << v << " ";
  }
  cout << endl;

  cout << "------------" << endl;

  const auto res2 = calcLocalCorrelationCoefficient(fields, 100, 1, 1);
  for (auto v : res2) {
    cout << v << " ";
  }
  cout << endl;

  return 0;
}
