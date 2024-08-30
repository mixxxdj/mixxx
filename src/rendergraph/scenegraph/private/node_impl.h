#pragma once

#include <QSGNode>

#include "rendergraph/node.h"

class rendergraph::NodeImplBase {
  public:
    NodeImplBase(Node* pOwner)
            : m_pOwner(pOwner) {
    }
    virtual ~NodeImplBase() = default;

    virtual QSGNode* sgNode() = 0;

    Node* owner() const {
        return m_pOwner;
    }

    void setUsePreprocess(bool value) {
        sgNode()->setFlag(QSGNode::UsePreprocess, value);
    }

    void onAppendChildNode(Node* pChild) {
        sgNode()->appendChildNode(pChild->impl().sgNode());
    }
    void onRemoveAllChildNodes() {
        sgNode()->removeAllChildNodes();
    }
    void onRemoveChildNode(Node* pChild) {
        sgNode()->removeChildNode(pChild->impl().sgNode());
    }

  private:
    Node* m_pOwner;
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
