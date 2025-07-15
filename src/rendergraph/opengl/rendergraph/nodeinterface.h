#pragma once

#include "backend/basenode.h"

namespace rendergraph {

template<class T_Node>
class NodeInterface : public T_Node {
  public:
    template<class T_AppendNode>
    T_AppendNode* appendChildNode(std::unique_ptr<T_AppendNode> pNode) {
        // Transfers ownership to this.
        T_AppendNode* pRawNode = pNode.release();
        // Note: Ideally we would use unique_ptrs internally, but
        // Qt uses raw pointers for QSGNode hierarchy. For simplicity
        // we mimic this.

        T_Node::appendChildNode(pRawNode);

        return pRawNode;
    }

    std::unique_ptr<BaseNode> detachChildNode(BaseNode* pNode) {
        // After removeChildNode, the caller has the responsibility
        // to deal with the child node. By returning a unique_ptr
        // we preoprtly transfer ownership to the caller (which
        // can result in deleting pNode if the caller doesn't
        // take the unique_ptr).
        T_Node::removeChildNode(pNode);
        return std::unique_ptr<BaseNode>(pNode);
    }
};

} // namespace rendergraph
