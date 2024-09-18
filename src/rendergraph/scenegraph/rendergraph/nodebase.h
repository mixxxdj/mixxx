#pragma once

#include <memory>

class QSGNode;

namespace rendergraph {
class NodeBase;
} // namespace rendergraph

class rendergraph::NodeBase {
  public:
    NodeBase();
    virtual ~NodeBase();

    void appendChildNode(std::unique_ptr<NodeBase>&& pChild);
    std::unique_ptr<NodeBase> removeAllChildNodes();
    std::unique_ptr<NodeBase> removeChildNode(NodeBase* pChild);

    NodeBase* parent() const {
        return m_pParent;
    }
    NodeBase* firstChild() const {
        return m_pFirstChild.get();
    }
    NodeBase* lastChild() const {
        return m_pLastChild;
    }
    NodeBase* nextSibling() const {
        return m_pNextSibling.get();
    }
    NodeBase* previousSibling() const {
        return m_pPreviousSibling;
    }

    void setUsePreprocess(bool value);

  private:
    QSGNode* sgNode();

    NodeBase* m_pParent{};
    std::unique_ptr<NodeBase> m_pFirstChild;
    NodeBase* m_pLastChild{};
    std::unique_ptr<NodeBase> m_pNextSibling;
    NodeBase* m_pPreviousSibling{};
};
