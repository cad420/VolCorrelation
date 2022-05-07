//
// Created by wyz on 2022/4/29.
//
#include "Info/ClusteringDendrogramWidget.hpp"
#include "Info/ForceDirectedLayoutWidget.hpp"
#include "Info/MutualInformation.hpp"
#include "Info/ForceDirected.hpp"
#include <QApplication>
#include <fstream>
#include <QListWidget>
#include <QPushButton>
#include <QFileDialog>

using namespace::std;
struct VolumeData {
  std::vector<uint8_t> data;
  std::string name;
  std::vector<size_t> histogram;
  double entropy = 0.0;
};
class Widget: public QWidget{
public:
  Widget(){
    volume_list = new QListWidget(this);
    volume_list->setGeometry(50,50,300,300);
    load_volume_pb = new QPushButton("load",this);
    load_volume_pb->setGeometry(50,400,100,20);
    compute_pb = new QPushButton("compute",this);
    compute_pb->setGeometry(250,400,100,20);
    connect(load_volume_pb,&QPushButton::clicked,this,[this](){
      auto path = QFileDialog::getOpenFileNames(this,
                                               QStringLiteral("Load Volume"),
                                               QStringLiteral("E:/Volume/"),
                                               QStringLiteral("(*.*)"));
      for(auto& p:path){
        if(!p.isEmpty()){
          loadVolume(p.toStdString());
        }
      }
    });
    connect(compute_pb,&QPushButton::clicked,this,[this](){
      draw();
    });
    dendrogram = new Info::ClusteringDendrogram();
    force_directed_layout = new Info::ForceDirectedLayout();

  }
  void loadVolume(const std::string& path){
    std::ifstream in(path,std::ios::binary);
    if(!in.is_open()){
      std::cerr<<"failed to open"<<std::endl;
      exit(1);
    }
    auto p = path.find_last_of("/");
    auto volume_name = path.substr(p+1);
    volume_list->addItem(QString(volume_name.c_str()));
    volumes.emplace_back();
    auto& volume = volumes.back();
    volume.data.resize(total);
    volume.name = volume_name.substr(0,volume_name.length() - 4);

    //if not uint8 should call Info::ConvertData to convert
    in.read(reinterpret_cast<char*>(volume.data.data()),total);

    volume.histogram = Info::CountValue(volume.data);
    volume.entropy = Info::CalculateEntropy(volume.histogram,total);
  }
  void clear(){
    volumes.clear();
    volume_list->clear();
  }
  void draw(){
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

    const auto k = static_cast<size_t>(volumes.size()-1);
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
  }
private:

  QListWidget* volume_list;
  QPushButton* load_volume_pb;
  QPushButton* compute_pb;
  Info::ClusteringDendrogram* dendrogram;
  Info::ForceDirectedLayout* force_directed_layout;
  std::vector<VolumeData> volumes;
  const int volume_x = 500, volume_y = 500, volume_z = 100;
  const size_t total = (size_t)volume_x * volume_y * volume_z;
};

int main(int argc,char** argv){
  QApplication app(argc,argv);

  auto w = new Widget();
  w->setFixedSize(400,500);
  w->show();

  return app.exec();
}