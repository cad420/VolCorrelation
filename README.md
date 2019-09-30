# Correlations in Multifield Scalar Data

## Gradient Similarity Measure

An implementation based on [Multifield-Graphs: An Approach to Visualizing Correlations in Multifield Scalar Data](https://ieeexplore.ieee.org/document/4015447/) section 3.1.1.

**NOT TESTED**

Usage:
```c++
std::vector<float> VolCorrelation::calculateGradientSimilarity(
  const std::vector<T *> fields,
  uint32_t width,
  uint32_t height,
  uint32_t depth,
  int sensitivity = 2
);
```