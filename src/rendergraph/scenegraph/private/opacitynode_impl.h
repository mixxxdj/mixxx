#pragma once

#include "node_impl.h"
#include "rendergraph/opacitynode.h"

class rendergraph::OpacityNode::Impl : public QSGOpacityNode, public rendergraph::NodeImplBase {
  public:
    Impl(OpacityNode* pOwner)
            : NodeImplBase(pOwner) {
    }

    QSGNode* sgNode() override {
        return this;
    }

    void preprocess() override {
        owner()->preprocess();
    }

    bool isSubtreeBlocked() const override {
        return QSGOpacityNode::isSubtreeBlocked() || owner()->isSubtreeBlocked();
    }
};
