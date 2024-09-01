#include "rendergraph/opacitynode.h"

#include "opacitynode_impl.h"

using namespace rendergraph;

OpacityNode::OpacityNode(NodeImplBase* pImpl)
        : Node(pImpl) {
}

OpacityNode::OpacityNode()
        : OpacityNode(new OpacityNode::Impl(this)) {
}

OpacityNode::~OpacityNode() = default;

void OpacityNode::setOpacity(float opacity) {
    static_cast<OpacityNode::Impl&>(impl()).setOpacity(opacity);
}
