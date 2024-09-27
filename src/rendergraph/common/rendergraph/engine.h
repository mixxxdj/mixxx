#pragma once

#include <memory>
#include <vector>

#include "rendergraph/node.h"

namespace rendergraph {
class Engine;
} // namespace rendergraph

class rendergraph::Engine {
  public:
    Engine(std::unique_ptr<TreeNode> pNode);
    void initialize();
    void render();
    void resize(int w, int h);
    void preprocess();
    void addToEngine(TreeNode* pNode);

  private:
    void initialize(TreeNode* pNode);
    void render(TreeNode* pNode);
    void resize(TreeNode* pNode, int, int);

    const std::unique_ptr<TreeNode> m_pTopNode;
    std::vector<TreeNode*> m_pPreprocessNodes;
    std::vector<TreeNode*> m_pInitializeNodes;
};
