#include "rendergraph/treenode.h"

#include "backend/basenode.h"

using namespace rendergraph;

void TreeNode::appendChildNode(std::unique_ptr<TreeNode>&& pChild) {
    auto pChildRawPtr = pChild.get();
    if (m_pLastChild) {
        pChild->m_pPreviousSibling = m_pLastChild;
        m_pLastChild->m_pNextSibling = std::move(pChild);
    } else {
        m_pFirstChild = std::move(pChild);
    }
    m_pLastChild = pChildRawPtr;
    m_pLastChild->m_pParent = this;

    onAppendChildNode(pChildRawPtr);
}

std::unique_ptr<TreeNode> TreeNode::removeAllChildNodes() {
    onRemoveAllChildNodes();

    m_pLastChild = nullptr;
    TreeNode* pChild = m_pFirstChild.get();
    while (pChild) {
        pChild->m_pParent = nullptr;
        pChild = pChild->m_pNextSibling.get();
    }
    return std::move(m_pFirstChild);
}

std::unique_ptr<TreeNode> TreeNode::removeChildNode(TreeNode* pChild) {
    onRemoveChildNode(pChild);

    std::unique_ptr<TreeNode> pRemoved;
    if (pChild == m_pFirstChild.get()) {
        pRemoved = std::move(m_pFirstChild);
        m_pFirstChild = std::move(pChild->m_pNextSibling);
    } else {
        pRemoved = std::move(pChild->m_pPreviousSibling->m_pNextSibling);
        pChild->m_pPreviousSibling->m_pNextSibling = std::move(pChild->m_pNextSibling);
        pChild->m_pPreviousSibling = nullptr;
    }
    if (pChild == m_pLastChild) {
        m_pLastChild = nullptr;
    }
    pChild->m_pParent = nullptr;
    return pRemoved;
}
