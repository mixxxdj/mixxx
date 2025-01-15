#pragma once

#include "backend/basenode.h"
#include "rendergraph/assert.h"

namespace rendergraph {

template<class T_Node>
class NodeInterface : public T_Node {
  public:
    void appendChildNode(std::unique_ptr<BaseNode> pNode) {
        BaseNode* pRawNode = pNode.release();
        pRawNode->setFlag(QSGNode::OwnedByParent, true);
        T_Node::appendChildNode(pRawNode);
        DEBUG_ASSERT(pRawNode->flags() & QSGNode::OwnedByParent);
    }
    std::unique_ptr<BaseNode> detachChildNode(BaseNode* pNode) {
        DEBUG_ASSERT(pNode->flags() & QSGNode::OwnedByParent);
        pNode->setFlag(QSGNode::OwnedByParent, false);
        T_Node::removeChildNode(pNode);
        DEBUG_ASSERT(!pNode->flags() & QSGNode::OwnedByParent);
        return std::unique_ptr<BaseNode>(pNode);
    }
};

} // namespace rendergraph
