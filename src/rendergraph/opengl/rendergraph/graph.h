#pragma once

#include <memory>
#include <vector>

namespace rendergraph {
class Graph;
class Node;
} // namespace rendergraph

class rendergraph::Graph {
  public:
    Graph(std::unique_ptr<Node> node);
    ~Graph();
    void initialize();
    void render();
    void resize(int w, int h);
    void preprocess();
    void addToGraph(Node* pNode);

  private:
    void render(Node* pNode);
    void resize(Node* pNode, int, int h);

    const std::unique_ptr<Node> m_pTopNode;
    std::vector<Node*> m_pPreprocessNodes;
    std::vector<Node*> m_pInitializeNodes;
};
