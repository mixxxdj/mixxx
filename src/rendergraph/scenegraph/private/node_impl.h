#pragma once

#include <QSGNode>

#include "rendergraph/node.h"

class rendergraph::NodeImplBase {
  public:
    NodeImplBase(Node* pOwner)
            : m_pOwner(pOwner) {
    }
    virtual ~NodeImplBase() = default;

    void appendChildNode(std::unique_ptr<Node> pChild) {
        sgNode()->appendChildNode(pChild->impl().sgNode());
        m_pChildren.emplace_back(std::move(pChild));
    }
    void removeAllChildNodes() {
        sgNode()->removeAllChildNodes();
        m_pChildren.clear();
    }

    Node* lastChild() const {
        return m_pChildren.back().get();
    }

    virtual QSGNode* sgNode() = 0;

    Node* owner() const {
        return m_pOwner;
    }

    void setUsePreprocess(bool value) {
        sgNode()->setFlag(QSGNode::UsePreprocess, value);
    }

  private:
    Node* m_pOwner;
    std::vector<std::unique_ptr<Node>> m_pChildren;
};

class rendergraph::Node::Impl : public QSGNode, public rendergraph::NodeImplBase {
  public:
    Impl(Node* pOwner)
            : NodeImplBase(pOwner) {
    }
    QSGNode* sgNode() override {
        return this;
    }
    bool isSubtreeBlocked() const override {
        return owner()->isSubtreeBlocked();
    }
    void preprocess() override {
        owner()->preprocess();
    }
};
