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
#include <QApplication>
#include <fstream>
using namespace::std;
int main(int argc,char** argv){
  QApplication app(argc,argv);

  // create QT widget
  auto dendrogram = new Info::ClusteringDendrogram();
  auto force_directed_layout = new Info::ForceDirectedLayout();

  // prepare volumes and histogram
  struct VolumeData {
    std::vector<uint8_t> data;
    std::string name;
    std::vector<size_t> histogram;
    double entropy = 0.0;
  };
  std::vector<VolumeData> volumes;
  // ...
  int volume_x = 500, volume_y = 500, volume_z = 100;
  size_t total = (size_t)volume_x * volume_y * volume_z;
  std::vector<std::string> volume_names = {
      "_SPEEDc21.raw","Pf21.raw","QCLOUDf21.raw","QVAPORf21.raw","TCc21.raw"
  };
  std::string data_path = "E:/Volume/buffer_data21/";
  int volume_count = volume_names.size();
  for(int i = 0; i < volume_count; i++){
    volumes.emplace_back();
    auto& volume = volumes.back();
    volume.data.resize(total);
    volume.name = volume_names[i].substr(0,volume_names[i].length() - 4);
    std::ifstream in(data_path + volume_names[i],std::ios::binary);
    //if not uint8 should call Info::ConvertData to convert
    in.read(reinterpret_cast<char*>(volume.data.data()),total);
    if(!in.is_open()){
      std::cerr<<"failed to open"<<std::endl;
      exit(1);
    }
    volume.histogram = Info::CountValue(volume.data);
    volume.entropy = Info::CalculateEntropy(volume.histogram,total);
  }

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
          first.data.data(), second.data.data(), first.histogram, second.histogram, total);
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

  const auto k = static_cast<size_t>(4);
  auto cluster = make_unique<Info::HierarchicalCluster>();
  cluster->process(distances, k);
  auto leaves = cluster->getLeaves();

  // prepare colors
//  auto colors = rndColors(k);
  std::vector<QColor> colors(k);
  for(auto& color:colors){
    color.setRgb(rand()%255,rand()%255,rand()%255);
  }
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
  force_directed_layout->init(particles, entropys, leaves, distances, colors,k);
  force_directed_layout->update();
  force_directed_layout->show();

  dendrogram->init(std::move(cluster), colors, names);
  dendrogram->update();
  dendrogram->show();
  return app.exec();
}
```