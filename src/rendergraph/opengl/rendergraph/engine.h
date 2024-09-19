#pragma once

#include <memory>
#include <vector>

#include "rendergraph/node.h"

namespace rendergraph {
class Engine;
} // namespace rendergraph

class rendergraph::Engine {
  public:
    Engine(std::unique_ptr<Node> node);
    void initialize();
    void render();
    void resize(int w, int h);
    void preprocess();
    void addToEngine(BaseNode* pNode);

  private:
    void initialize(BaseNode* pNode);
    void render(BaseNode* pNode);
    void resize(BaseNode* pNode, int, int);

    const std::unique_ptr<Node> m_pTopNode;
    std::vector<BaseNode*> m_pPreprocessNodes;
    std::vector<BaseNode*> m_pInitializeNodes;
};
