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

### Correlation Based on Information Theory

An implementation based on [An Information-Aware Framework for Exploring Multivariate Data Sets](https://ieeexplore.ieee.org/abstract/document/6634187).

**NOT TESTED**

Usage:

```c++
#include "Info/ClusteringDendrogramWidget.hpp"
#include "Info/ForceDirectedLayoutWidget.hpp"
#include "Info/MutualInformation.hpp"
#include "Info/ForceDirected.hpp"

// create QT widget
auto dendrogram = new Info::ClusteringDendrogram();
auto force_directed_layout = new Info::ForceDirectedLayout();

// prepare volumes and histogram
struct VolumeData {
  uint8_t *data = nullptr;
  std::string name;
  std::vector<int> histogram;
  double entropy = 0.0;
};
std::vector<VolumeData> volumes;
// ...

// prepare container to hold mutual informations and distance
vector<vector<double>> MI(volumes.size()), distances(volumes.size());
for (size_t i = 0; i < volumes.size(); i++) {
    MI[i].resize(volumes.size(), 0);
    distances[i].resize(volumes.size(), 0);
}

// calculate mutual information
auto min = FLT_MAX;
for (size_t i = 0; i < volumes.size(); i++) {
    auto &first = volumes[i];
    for (size_t j = i + 1; j < volumes.size(); j++) {
        auto &second = volumes[j];
        auto I = Info::CalculateMutationInformation(
          first.data, second.data, first.histogram, second.histogram, total);
        MI[i][j] = I;
        if (I < min && (0 != min)) {
            min = I;
        }
    }
}

// calculate distance, use reverse of MI and map them to 0~100
for (size_t i = 0; i < volumes.size(); i++) {
    for (size_t j = i + 1; j < volumes.size(); j++) {
        auto I = MI[i][j];
        if (I == 0) {
            distances[i][j] = 100.0;
        } else {
            distances[i][j] = 100.0 * min / I;
        }
    }
}

const auto k = static_cast<size_t>(atts->GetClusterSize());
auto cluster = make_unique<Info::HierarchicalCluster>();
cluster->process(distances, k);
auto leaves = cluster->getLeaves();

// prepare colors
auto colors = rndColors(k);
auto names = vector<string>();
for (auto &volume : volumes) {
    names.push_back(volume.name);
}

auto entropys = vector<double>();
for (auto &volume : volumes) {
    entropys.push_back(volume.entropy);
}

// calculate particles in force directed layout
const auto width = 400;
const auto height = 600;
auto particles = Info::CalculateForceDirected(distances, width, height);

// display
force_directed_layout->init(particles, entropys, leaves, distances, colors, k);
force_directed_layout->update();
force_directed_layout->show();

dendrogram->init(std::move(cluster), colors, names);
dendrogram->update();
dendrogram->show();
```