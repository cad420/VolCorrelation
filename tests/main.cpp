#include "VolCorrelation/GradientSimilarityMeasure.hpp"
#include "VolCorrelation/LocalCorrelationCoefficient.hpp"
#include "Info/MutualInformation.hpp"
#include <iostream>
#include <vector>
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QProcess>
#include <QFileDialog>
#include <iostream>
#include <fstream>
using namespace std;
using namespace VolCorrelation;
class Widget: public QWidget {
public:
  Widget(){
    volume_list = new QListWidget(this);
    volume_list->setGeometry(50,50,300,300);
    load_volume_pb = new QPushButton("load",this);
    load_volume_pb->setGeometry(50,400,80,20);
    clear_volume_pb = new QPushButton("clear",this);
    clear_volume_pb->setGeometry(150,400,80,20);
    compute_pb1 = new QPushButton("梯度",this);
    compute_pb1->setGeometry(250,400,80,20);
    compute_pb2 = new QPushButton("相关性系数",this);
    compute_pb2->setGeometry(250,430,80,20);
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
    connect(clear_volume_pb,&QPushButton::clicked,this,[this](){
      volume_list->clear();
      volumes.clear();
    });
    connect(compute_pb1,&QPushButton::clicked,this,[this](){
      compute1();
      draw();
    });
    connect(compute_pb2,&QPushButton::clicked,this,[this](){
      compute2();
      draw();
    });
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
    volume.resize(total,0);
    in.read(reinterpret_cast<char*>(volume.data()),volume.size());
    in.close();
  }
  void compute1(){
    vector<uint8_t*> fields;
    for(auto& volume:volumes){
      fields.emplace_back(volume.data());
    }
    const auto res = calculateGradientSimilarity(fields,volume_x,volume_y,volume_z);
    auto ret = Info::ConvertData(res);
    ofstream out("tmp.raw",std::ios::binary);
    out.write(reinterpret_cast<char*>(ret.data()),ret.size());
    out.close();
  }
  void compute2(){
    vector<uint8_t*> fields;
    for(auto& volume:volumes){
      fields.emplace_back(volume.data());
    }
    const auto res = calcLocalCorrelationCoefficient(fields,volume_x,volume_y,volume_z);
    auto ret = Info::ConvertData(res);
    ofstream out("tmp.raw",std::ios::binary);
    out.write(reinterpret_cast<char*>(ret.data()),ret.size());
    out.close();
  }
  void draw(){
    bool e = QProcess::startDetached("VolumeRender.exe",{"--raw_file=tmp.raw","--raw_x=500","--raw_y=500","--raw_z=100","--raw_data_type=uint8","--tf_file=tf.json"});
    if(!e){
      std::cerr<<"start VolumeRender.exe failed"<<std::endl;
      exit(1);
    }
  }
private:
  QListWidget* volume_list;
  QPushButton* load_volume_pb;
  QPushButton* clear_volume_pb;
  QPushButton* compute_pb1;
  QPushButton* compute_pb2;
  vector<vector<uint8_t>> volumes;
  const int volume_x = 500, volume_y = 500, volume_z = 100;
  const size_t total = (size_t)volume_x * volume_y * volume_z;
};
int main(int argc,char** argv) {
  QApplication app(argc,argv);

  auto w = new Widget();
  w->setFixedSize(400,500);
  w->show();

  return app.exec();
}
