#include "rendergraph/nodebase.h"

#include <QSGNode>

using namespace rendergraph;

NodeBase::NodeBase() = default;
NodeBase::~NodeBase() = default;

QSGNode* NodeBase::sgNode() {
    return dynamic_cast<QSGNode*>(this);
}

void NodeBase::setUsePreprocess(bool value) {
    sgNode()->setFlag(QSGNode::UsePreprocess, value);
}

void NodeBase::appendChildNode(std::unique_ptr<NodeBase>&& pChild) {
    sgNode()->appendChildNode(pChild->sgNode());

    auto pChildRawPtr = pChild.get();
    if (m_pLastChild) {
        pChild->m_pPreviousSibling = m_pLastChild;
        m_pLastChild->m_pNextSibling = std::move(pChild);
    } else {
        m_pFirstChild = std::move(pChild);
    }
    m_pLastChild = pChildRawPtr;
    m_pLastChild->m_pParent = this;
}

std::unique_ptr<NodeBase> NodeBase::removeAllChildNodes() {
    sgNode()->removeAllChildNodes();

    m_pLastChild = nullptr;
    NodeBase* pChild = m_pFirstChild.get();
    while (pChild) {
        pChild->m_pParent = nullptr;
        pChild = pChild->m_pNextSibling.get();
    }
    return std::move(m_pFirstChild);
}

std::unique_ptr<NodeBase> NodeBase::removeChildNode(NodeBase* pChild) {
    sgNode()->removeChildNode(pChild->sgNode());

    std::unique_ptr<NodeBase> pRemoved;
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
