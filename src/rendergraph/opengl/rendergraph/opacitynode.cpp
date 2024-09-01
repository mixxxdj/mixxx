#include "rendergraph/opacitynode.h"

using namespace rendergraph;

OpacityNode::OpacityNode() = default;

OpacityNode::~OpacityNode() = default;

void OpacityNode::setOpacity(float opacity) {
    m_opacity = opacity;
}

bool OpacityNode::isSubtreeBlocked() const {
    return m_opacity == 0.f;
}
