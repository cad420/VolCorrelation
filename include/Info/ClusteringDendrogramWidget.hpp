#pragma once
#include "HierarchicalCluster.hpp"
#include "QWidget"
#include "QtGui"
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>

namespace Info {

inline void draw_tree(QPainter &painter, const Info::Node *node, int &offset,
                     int width, int yBegin, int fontSize,
                     std::vector<QColor> &colors,
                     std::vector<std::string> &names) {
  std::map<const Info::Node *, std::array<int, 2>> positions;
  std::map<const Info::Node *, bool> history;
  const auto blackPen = QPen(QBrush(Qt::black, Qt::SolidPattern), 3);

  auto font = painter.font();

  std::vector<const Info::Node *> stack;
  stack.push_back(node);
  while (!stack.empty()) {
    auto node = stack.back();

    if (positions.find(node) == positions.end()) {
      positions.emplace(node, std::array<int, 2>{0, 0});
    }

    if (node->isLeaf) {
      auto &position = positions[node];
      position[0] = offset;
      position[1] = yBegin;
      auto pen = painter.pen();
      painter.setPen(blackPen);
      font.setPointSize(fontSize);
      painter.setFont(font);
      painter.drawText(position[0] - fontSize / 2, position[1] + 2 * fontSize,
                       std::to_string(node->id + 1).c_str());
      font.setPointSize(fontSize / 2);
      painter.setFont(font);
      painter.drawText(position[0] - fontSize, position[1] + 4 * fontSize,
                       names[node->id].c_str());
      painter.setPen(pen);
      offset += width;
      history.emplace(node, true);
      stack.pop_back();
      continue;
    }

    if (history.find(node->left) == history.end()) {
      stack.push_back(node->left);
      continue;
    }

    if (history.find(node->right) == history.end()) {
      stack.push_back(node->right);
      continue;
    }

    auto &position = positions[node];
    auto leftPos = positions[node->left];
    auto rightPos = positions[node->right];

    position[0] = (leftPos[0] + rightPos[0]) / 2;
    position[1] = yBegin - static_cast<int>(std::sqrt(node->distance * 10) * 10 + 10);

    if (node->belong != -1) {
      auto color = colors[node->belong];
      auto pen = QPen(QBrush(color, Qt::SolidPattern), 3);
      painter.setPen(pen);
    } else {
      painter.setPen(blackPen);
    }

    painter.drawLine(leftPos[0], leftPos[1], leftPos[0], position[1]);
    painter.drawLine(rightPos[0], rightPos[1], rightPos[0], position[1]);
    painter.drawLine(leftPos[0], position[1], rightPos[0], position[1]);
    history.emplace(node, true);
    stack.pop_back();
  }
}

class ClusteringDendrogram : public QWidget {
public:
  ClusteringDendrogram() : cluster(nullptr), colors(), names() {}
  void init(std::unique_ptr<HierarchicalCluster> cluster,
            const std::vector<QColor> &colors,
            const std::vector<std::string> &names) {
    this->cluster = std::move(cluster);
    this->colors = colors;
    this->names = names;
  }

protected:
  void paintEvent(QPaintEvent *event) {
    if (cluster->getRoot() == nullptr) {
      return;
    }

    const auto fontSize = 16;
    int marginX[] = {40, 40};
    int offset = marginX[0];
    const auto width = (this->width() - marginX[0] - marginX[1]) /
                       (static_cast<int>(cluster->getRoot()->count) - 1);
    const auto yBegin = this->height() * 3 / 4;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QFont font = painter.font();
    font.setPointSize(fontSize);
    painter.setFont(font);

    draw_tree(painter, cluster->getRoot(), offset, width, yBegin, fontSize,
             colors, names);

    // draw axis
    const auto axisPen = QPen(QBrush(Qt::black, Qt::SolidPattern), 3);
    painter.setPen(axisPen);
    painter.drawLine(marginX[0], yBegin, this->width() - marginX[1], yBegin);
    painter.end();
  }

private:
  std::unique_ptr<HierarchicalCluster> cluster;
  std::vector<QColor> colors;
  std::vector<std::string> names;
};
} // namespace Info
