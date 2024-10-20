#include "backend/basenode.h"

#include "rendergraph/assert.h"
#include "rendergraph/engine.h"

using namespace rendergraph;

BaseNode::~BaseNode() {
    DEBUG_ASSERT(m_pParent == nullptr);
    DEBUG_ASSERT(m_pEngine == nullptr);

    while (m_pFirstChild) {
        std::unique_ptr<BaseNode> pChild(m_pFirstChild);
        removeChildNode(pChild.get());
    }
}

void BaseNode::appendChildNode(BaseNode* pChild) {
    if (m_pLastChild) {
        pChild->m_pPreviousSibling = m_pLastChild;
        m_pLastChild->m_pNextSibling = pChild;
    } else {
        m_pFirstChild = pChild;
    }
    m_pLastChild = pChild;
    m_pLastChild->m_pParent = this;

    DEBUG_ASSERT(m_pLastChild->m_pNextSibling == nullptr);

    if (m_pEngine) {
        m_pEngine->add(pChild);
    }
}

void BaseNode::removeAllChildNodes() {
    while (m_pFirstChild) {
        removeChildNode(m_pFirstChild);
    }
}

void BaseNode::removeChildNode(BaseNode* pChild) {
    if (pChild == m_pFirstChild) {
        m_pFirstChild = pChild->m_pNextSibling;
    } else {
        pChild->m_pPreviousSibling->m_pNextSibling = pChild->m_pNextSibling;
    }

    if (pChild == m_pLastChild) {
        m_pLastChild = nullptr;
    }

    if (pChild->m_pEngine) {
        pChild->m_pEngine->remove(pChild);
    }

    pChild->m_pNextSibling = nullptr;
    pChild->m_pPreviousSibling = nullptr;
    pChild->m_pParent = nullptr;
}
