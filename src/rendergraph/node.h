#pragma once

#include <list>
#include <memory>

namespace rendergraph {
class Node;
class NodeImplBase;
} // namespace rendergraph

class rendergraph::Node {
  public:
    class Impl;

    Node();

    virtual ~Node();

    void appendChildNode(std::unique_ptr<Node>&& pChild) {
        auto pChildRawPtr = pChild.get();
        if (m_pLastChild) {
            pChild->m_pPreviousSibling = m_pLastChild;
            m_pLastChild->m_pNextSibling = std::move(pChild);
        } else {
            m_pFirstChild = std::move(pChild);
        }
        m_pLastChild = pChildRawPtr;
        m_pLastChild->m_pParent = this;
        onAppendChildNode(m_pLastChild);
    }
    std::unique_ptr<Node> removeAllChildNodes() {
        onRemoveAllChildNodes();
        m_pLastChild = nullptr;
        Node* pChild = m_pFirstChild.get();
        while (pChild) {
            pChild->m_pParent = nullptr;
            pChild = pChild->m_pNextSibling.get();
        }
        return std::move(m_pFirstChild);
    }
    std::unique_ptr<Node> removeChildNode(Node* pChild) {
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
    Node* parent() const {
        return m_pParent;
    }
    Node* firstChild() const {
        return m_pFirstChild.get();
    }
    Node* lastChild() const {
        return m_pLastChild;
    }
    Node* nextSibling() const {
        return m_pNextSibling.get();
    }
    Node* previousSibling() const {
        return m_pPreviousSibling;
    }
    NodeImplBase& impl() const;

    virtual bool isSubtreeBlocked() const {
        return false;
    }

    virtual void preprocess() {
    }

    void setUsePreprocess(bool value);

  protected:
    Node(NodeImplBase* impl);

  private:
    const std::unique_ptr<NodeImplBase> m_pImpl;
    Node* m_pParent{};
    std::unique_ptr<Node> m_pFirstChild;
    Node* m_pLastChild{};
    std::unique_ptr<Node> m_pNextSibling;
    Node* m_pPreviousSibling{};

    void onAppendChildNode(Node* pChild);
    void onRemoveChildNode(Node* pChild);
    void onRemoveAllChildNodes();
};
