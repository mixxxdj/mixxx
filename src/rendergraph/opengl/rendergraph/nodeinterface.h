#pragma once

#include "backend/basenode.h"

namespace rendergraph {

template<class T_Node>
class NodeInterface : public T_Node {
  public:
    void appendChildNode(std::unique_ptr<BaseNode> pNode) {
        BaseNode* pRawNode = pNode.release();
        T_Node::appendChildNode(pRawNode);
    }
    std::unique_ptr<BaseNode> detachChildNode(BaseNode* pNode) {
        T_Node::removeChildNode(pNode);
        return std::unique_ptr<BaseNode>(pNode);
    }
};

} // namespace rendergraph
