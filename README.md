# Correlations in Multifield Scalar Data

## Gradient Similarity Measure

An implementation based on [Multifield-Graphs: An Approach to Visualizing Correlations in Multifield Scalar Data](https://ieeexplore.ieee.org/document/4015447/) section 3.1.1.

**NOT TESTED**

Usage:
```c++
#include "VolCorrelation/GradientSimilarityMeasure.hpp"

std::vector<float> VolCorrelation::calculateGradientSimilarity(
  const std::vector<T *> fields,
  uint32_t width,
  uint32_t height,
  uint32_t depth,
  int sensitivity = 2
);
```

## Local Correlation Coefficient

An implementation based on [Multifield-Graphs: An Approach to Visualizing Correlations in Multifield Scalar Data](https://ieeexplore.ieee.org/document/4015447/) section 3.1.2.

**NOT TESTED**

Usage:
```c++
#include "VolCorrelation/LocalCorrelationCoefficient.hpp"

template <typename T, typename ResultType = float>
std::vector<ResultType> VolCorrelation::calcLocalCorrelationCoefficient(
  const std::vector<T *> &fields,
  uint32_t width,
  uint32_t height,
  uint32_t depth,
  int windowSize = 3
);
```
