#pragma once

#include "QWidget"
#include "QtGui"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>

namespace Info {
struct ForceCircle {
  size_t id;
  double x;
  double y;
  double radius;
  bool selected;
  bool isMax;
  double entropy;
  int belong;
  ForceCircle() = default;
  ForceCircle(size_t id, double x, double y, double radius, bool isMax,
              double entropy, int belong)
      : id(id), x(x), y(y), radius(radius), selected(false), isMax(isMax),
        entropy(entropy), belong(belong) {}
};

struct ForceCluster {
  double max = 0.0;
  QColor color;
  std::vector<size_t> nodes;
};

inline double distance(double x1, double y1, double x2, double y2) {
  return std::sqrt(std::powf(x1 - x2, 2) + std::powf(y1 - y2, 2));
}

class ForceDirectedLayout : public QWidget {
public:
  std::vector<ForceCluster> clusters;
  std::vector<ForceCircle> circles;
  ForceCircle* selected = nullptr;
  std::vector<std::vector<double>> distances;
  ForceDirectedLayout(): clusters(), circles(), distances() {}
  void init(const std::vector<std::array<double, 2>> &particles,
            const std::vector<double> &entropys,
            const std::vector<Info::Node *> &nodes,
            const std::vector<std::vector<double>> &distances,
            const std::vector<QColor> &colors, int k) {
    this->clusters.resize(k);
    this->distances = distances;

    for (size_t i = 0; i < k; i++) {
      clusters[i].max = 0.0;
      clusters[i].color = colors[i];
    }

    for (auto &node : nodes) {
      auto &cluster = clusters[node->belong];
      if (entropys[node->id] > cluster.max) {
        cluster.max = entropys[node->id];
      }
    }
    int node_count = nodes.size();
    float r = std::min(QWidget::height(),QWidget::width()) * 0.5 * 0.6;
    float degree = 2 * M_PI / node_count;
    int i = 0;
    for (auto &node : nodes) {
      const auto entropy = entropys[node->id];
      const auto &particle = particles[node->id];
      const auto clusterId = node->belong;
      auto &cluster = clusters[clusterId];
      const auto isMax = entropy == cluster.max;
      auto x = QWidget::width() * 0.5 + r * sin(degree * i);
      auto y = QWidget::height() * 0.5 + r * cos(degree * i++);
      circles.emplace_back(node->id, x, y,
                           entropy / cluster.max * 15 + 10, isMax, entropy,
                           clusterId);
      cluster.nodes.push_back(circles.size() - 1);
    }
  }

protected:
  void paintEvent(QPaintEvent *event) override {
    if (clusters.empty()) {
      return ;
    }

    const auto width = this->width();
    const auto height = this->height();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QFont font = painter.font();
    const auto fontSize = 12;
    font.setPointSize(fontSize);
    painter.setFont(font);

    painter.setPen(Qt::gray);
    for (size_t i = 0; i < circles.size(); i++) {
      for (size_t j = i + 1; j < circles.size(); j++) {
        painter.drawLine(circles[i].x, circles[i].y, circles[j].x,
                         circles[j].y);
      }
    }

    for (size_t i = 0; i < clusters.size(); i++) {
      const auto &cluster = clusters[i];
      for (auto idx : cluster.nodes) {
        auto &c = circles[idx];
        if (c.selected) {
          painter.setBrush(QBrush(Qt::black, Qt::SolidPattern));
          painter.setPen(Qt::white);
        }
        else {
//          if (c.isMax) {
//            painter.setBrush(QBrush(Qt::black, Qt::SolidPattern));
//            painter.drawEllipse(c.x - c.radius - 5, c.y - c.radius - 5,
//                                c.radius * 2 + 10, c.radius * 2 + 10);
//          }
          painter.setBrush(QBrush(cluster.color, Qt::SolidPattern));
          painter.setPen(Qt::black);
        }
        painter.drawEllipse(c.x - c.radius, c.y - c.radius, c.radius * 2,
                            c.radius * 2);
        painter.drawText(QPoint(c.x - fontSize / 2, c.y + fontSize / 2),
                         std::to_string(c.id + 1).c_str());
      }
    }
  }
  bool first;
  void mouseMoveEvent(QMouseEvent* event) override{
    const auto x = event->x();
    const auto y = event->y();
    static int last_x = 0;
    static int last_y = 0;


    if(first){
      last_x = x;
      last_y = y;
      first = false;
    }
    const auto dx = x - last_x;
    const auto dy = y - last_y;
    last_x = x;
    last_y = y;
    if(!selected) return;
    selected->x += dx;
    selected->y += dy;
    update();
  }
  void mouseReleaseEvent(QMouseEvent* event) override{
    selected = nullptr;
  }
  void mousePressEvent(QMouseEvent *event) override {
    const auto x = event->x();
    const auto y = event->y();
    auto find = false;
    selected = nullptr;
    for (auto i = circles.rbegin(); i != circles.rend(); i++) {
      auto &circle = *i;
      circle.selected = false;
      if (!find && distance(circle.x, circle.y, x, y) <= circle.radius) {
        find = true;
        circle.selected = true;
        selected = &circle;
      }
    }
    if(find){
      first = true;
    }
    if (!find) {
      for (int i = static_cast<int>(circles.size()) - 1; i >= 0; i--) {
        auto &circle = circles[i];
        const auto &cluster = clusters[circle.belong];
        circle.radius = circle.entropy / cluster.max * 20 + 10, 0.0;
        circle.isMax = circle.entropy == cluster.max;
      }
    }
    else {
      const auto &cluster = clusters[selected->belong];
      auto maxIdx = cluster.nodes[0];
      if (selected->id == circles[maxIdx].id && cluster.nodes.size() > 1) {
        maxIdx = cluster.nodes[1];
      }
      for (size_t idx = 0; idx < circles.size();idx++) {
        auto &circle = circles[idx];
        if (circle.id != selected->id) {
          auto first = circle.id < selected->id ? circle.id : selected->id;
          auto second = circle.id > selected->id ? circle.id : selected->id;
          circle.radius = distances[first][second] + 10;
          if (circles[maxIdx].radius < circle.radius) {
            maxIdx = idx;
          }
        } else {
//          circle.radius = 30;
          circle.radius = circle.entropy / cluster.max * 20 + 10, 0.0;
          circle.isMax = circle.entropy == cluster.max;
        }
      }

      for (auto idx : cluster.nodes) {
        auto &circle = circles[idx];
        if (idx == maxIdx) {
          circle.isMax = true;
        } else {
          circle.isMax = false;
        }
      }
    }

    update();
  }
};

}
