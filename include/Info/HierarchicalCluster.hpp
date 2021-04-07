#pragma once

#include "MutualInformation.hpp"
#include <array>
#include <map>
#include <utility>

namespace Info {

struct Node {
  size_t id = 0;
  bool isLeaf = true;
  double distance = 0.0;
  size_t count = 1;
  Node *left = nullptr;
  Node *right = nullptr;
  int belong = -1;
};

inline double calcDistance(Node *a, Node *b,
                           const std::vector<std::vector<double>> &distances) {
  if (a->isLeaf && b->isLeaf) {
    auto minIdx = a->id < b->id ? a->id : b->id;
    auto maxIdx = a->id < b->id ? b->id : a->id;
    return distances[minIdx][maxIdx];
  }

  auto distance = 0.0;

  if (a->isLeaf && !b->isLeaf) {
    distance += calcDistance(a, b->left, distances);
    distance += calcDistance(a, b->right, distances);
    return distance;
  }

  if (!a->isLeaf && b->isLeaf) {
    distance += calcDistance(a->left, b, distances);
    distance += calcDistance(a->right, b, distances);
    return distance;
  }

  distance += calcDistance(a->left, b->left, distances);
  distance += calcDistance(a->left, b->right, distances);
  distance += calcDistance(a->right, b->left, distances);
  distance += calcDistance(a->right, b->right, distances);
  return distance;
}

class HierarchicalCluster {
public:
  void process(const std::vector<std::vector<double>> &distances, size_t k) {
    std::vector<Node *> nodes(distances.size());
    for (size_t i = 0; i < nodes.size(); i++) {
      nodes[i] = new Node();
      nodes[i]->id = i;
    }

    size_t i = 0, j = 0;
    while (nodes.size() > 1) {
      auto closest = std::make_pair<size_t, size_t>(0, 0);
      double min = FLT_MAX;
      for (i = 0; i < nodes.size(); i++) {
        auto nodeA = nodes[i];
        for (j = i + 1; j < nodes.size(); j++) {
          auto nodeB = nodes[j];
          auto distance = calcDistance(nodeA, nodeB, distances);
          distance /= (nodeA->count * nodeB->count);
          if (distance < min) {
            min = distance;
            closest.first = i;
            closest.second = j;
          }
        }
      }
      auto nodeA = nodes[closest.first];
      auto nodeB = nodes[closest.second];
      // merge
      auto node = new Node();
      node->isLeaf = false;
      node->count = nodeA->count + nodeB->count;
      node->left = nodeA;
      node->right = nodeB;
      node->distance = min;
      nodes[closest.first] = node;
      nodes[closest.second] = nodes.back();
      nodes.pop_back();

      if (nodes.size() == k) {
        this->clusters.insert(clusters.end(), nodes.begin(), nodes.end());
      }
    }

    // traversal tree and mark cluster id for each node
    for (size_t i = 0; i < clusters.size(); i++) {
      std::map<Node *, bool> history;
      std::vector<Node *> stack;
      stack.push_back(clusters[i]);
      while (!stack.empty()) {
        auto node = stack.back();
        if (node->isLeaf) {
          node->belong = static_cast<int>(i);
          stack.pop_back();
          history.emplace(node, true);
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

        node->belong = static_cast<int>(i);
        history.emplace(node, true);
        stack.pop_back();
      }
    }

    this->root = nodes[0];
  }

  std::vector<Node *> getLeaves() const {
    std::map<Node *, bool> history;
    std::vector<Node *> stack;
    std::vector<Node *> leaves;

    stack.push_back(this->root);
    while (!stack.empty()) {
      auto node = stack.back();
      if (node->isLeaf) {
        leaves.push_back(node);
        stack.pop_back();
        history.emplace(node, true);
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

      history.emplace(node, true);
      stack.pop_back();
    }

    std::sort(leaves.begin(), leaves.end(),
              [](auto &a, auto &b) { return a->id < b->id; });
    return leaves;
  }

  std::vector<Node *> getClusters() const { return clusters; };

  Node *getRoot() const { return root; }

  ~HierarchicalCluster() {
    // free all nodes
    std::map<Node *, bool> history;
    std::vector<Node *> stack;

    stack.push_back(this->root);
    while (!stack.empty()) {
      auto node = stack.back();
      if (node->isLeaf) {
        delete node;
        stack.pop_back();
        history.emplace(node, true);
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

      history.emplace(node, true);
      delete node;
      stack.pop_back();
    }
  }

private:
  Node *root;
  std::vector<Node *> clusters;
};

} // namespace Info
