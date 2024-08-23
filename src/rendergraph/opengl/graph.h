#pragma once

#include <memory>
#include <vector>

namespace rendergraph {
class Graph;
class Node;
} // namespace rendergraph

class rendergraph::Graph {
  public:
    class Impl;
    Graph(std::unique_ptr<Node> node);
    ~Graph();
    Impl& impl() const;
    void initialize();
    void render();
    void resize(int w, int h);
    void preprocess();

  private:
    const std::unique_ptr<Node> m_pTopNode;
    std::vector<Node*> m_pPreprocessNodes;
};
