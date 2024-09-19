#include "rendergraph/node.h"

#include <QSGNode>

using namespace rendergraph;

void Node::setUsePreprocess(bool value) {
    sgNode()->setFlag(QSGNode::UsePreprocess, value);
}

void Node::appendChildNode(std::unique_ptr<Node>&& pChild) {
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

std::unique_ptr<Node> Node::removeAllChildNodes() {
    sgNode()->removeAllChildNodes();

    m_pLastChild = nullptr;
    Node* pChild = m_pFirstChild.get();
    while (pChild) {
        pChild->m_pParent = nullptr;
        pChild = pChild->m_pNextSibling.get();
    }
    return std::move(m_pFirstChild);
}

std::unique_ptr<Node> Node::removeChildNode(Node* pChild) {
    sgNode()->removeChildNode(pChild->sgNode());

    std::unique_ptr<Node> pRemoved;
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
