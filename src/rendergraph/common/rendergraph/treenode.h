#pragma once

#include <memory>

#include "backend/basenode.h"

namespace rendergraph {
class TreeNode;
} // namespace rendergraph

class rendergraph::TreeNode {
  public:
    TreeNode(rendergraph::BaseNode* pBackendNode)
            : m_pBackendNode(pBackendNode) {
    }
    virtual ~TreeNode() = default;

    void appendChildNode(std::unique_ptr<TreeNode>&& pChild);
    std::unique_ptr<TreeNode> removeAllChildNodes();
    std::unique_ptr<TreeNode> removeChildNode(TreeNode* pChild);

    TreeNode* parent() const {
        return m_pParent;
    }
    TreeNode* firstChild() const {
        return m_pFirstChild.get();
    }
    TreeNode* lastChild() const {
        return m_pLastChild;
    }
    TreeNode* nextSibling() const {
        return m_pNextSibling.get();
    }
    TreeNode* previousSibling() const {
        return m_pPreviousSibling;
    }

    void setUsePreprocess(bool value);

    rendergraph::BaseNode* backendNode() {
        return m_pBackendNode;
    }

    virtual void initialize() {
    }
    virtual void resize(int, int) {
    }

  private:
    void onAppendChildNode(TreeNode* pChild);
    void onRemoveAllChildNodes();
    void onRemoveChildNode(TreeNode* pChild);

    rendergraph::BaseNode* m_pBackendNode;
    TreeNode* m_pParent{};
    std::unique_ptr<TreeNode> m_pFirstChild;
    TreeNode* m_pLastChild{};
    std::unique_ptr<TreeNode> m_pNextSibling;
    TreeNode* m_pPreviousSibling{};
};
