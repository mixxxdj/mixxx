#include "rendergraph/node.h"

#include "rendergraph/graph.h"

using namespace rendergraph;

Node::Node() = default;

Node::~Node() = default;

void Node::appendChildNode(std::unique_ptr<Node>&& pChild) {
    auto pChildRawPtr = pChild.get();
    if (m_pLastChild) {
        pChild->m_pPreviousSibling = m_pLastChild;
        m_pLastChild->m_pNextSibling = std::move(pChild);
    } else {
        m_pFirstChild = std::move(pChild);
    }
    m_pLastChild = pChildRawPtr;
    m_pLastChild->m_pParent = this;
    if (graph() != nullptr && graph() != pChildRawPtr->graph()) {
        graph()->addToGraph(pChildRawPtr);
    }
}
std::unique_ptr<Node> Node::removeAllChildNodes() {
    m_pLastChild = nullptr;
    Node* pChild = m_pFirstChild.get();
    while (pChild) {
        pChild->m_pParent = nullptr;
        pChild = pChild->m_pNextSibling.get();
    }
    return std::move(m_pFirstChild);
}
std::unique_ptr<Node> Node::removeChildNode(Node* pChild) {
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
