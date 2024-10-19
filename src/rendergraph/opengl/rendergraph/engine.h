#pragma once

#include <QMatrix4x4>
#include <memory>
#include <vector>

#include "rendergraph/node.h"

namespace rendergraph {
class Engine;
} // namespace rendergraph

class rendergraph::Engine {
  public:
    Engine(std::unique_ptr<TreeNode> pNode);
    void render();
    void resize(int w, int h);
    void preprocess();
    void add(TreeNode* pNode);
    const QMatrix4x4& matrix() const {
        return m_matrix;
    }

  private:
    void render(TreeNode* pNode);
    void resize(TreeNode* pNode, int, int);

    QMatrix4x4 m_matrix;
    const std::unique_ptr<TreeNode> m_pTopNode;
    std::vector<TreeNode*> m_pPreprocessNodes;
    std::vector<TreeNode*> m_pInitializeNodes;
};
