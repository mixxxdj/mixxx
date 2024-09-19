#pragma once

#include <QSGNode>
#include <memory>

namespace rendergraph {
class Node;
} // namespace rendergraph

class rendergraph::Node {
  public:
    // We can't derive from QSGNode, as we need to avoid ambiguous names and
    // the diamond problem, so for the base class we use encapsulation. The
    // derived classes (GeometryNode, OpacityNode) do arrive from Node and
    // from their Qt scene graph counterpart and pass their pointer to the
    // constructor.
    Node()
            : m_pSgNode(new QSGNode),
              m_ownSgNode(true) {
    }
    Node(QSGNode* sgNode)
            : m_pSgNode(sgNode),
              m_ownSgNode(false) {
    }
    virtual ~Node() {
        if (m_ownSgNode) {
            delete m_pSgNode;
        }
    }

    void appendChildNode(std::unique_ptr<Node>&& pChild);
    std::unique_ptr<Node> removeAllChildNodes();
    std::unique_ptr<Node> removeChildNode(Node* pChild);

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

    void setUsePreprocess(bool value);

    QSGNode* sgNode() {
        return m_pSgNode;
    }

  private:
    QSGNode* m_pSgNode;
    bool m_ownSgNode;
    Node* m_pParent{};
    std::unique_ptr<Node> m_pFirstChild;
    Node* m_pLastChild{};
    std::unique_ptr<Node> m_pNextSibling;
    Node* m_pPreviousSibling{};
};
