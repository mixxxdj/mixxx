#pragma once

#include "backend/baseopacitynode.h"
#include "rendergraph/nodeinterface.h"

namespace rendergraph {
class OpacityNode;
} // namespace rendergraph

class rendergraph::OpacityNode : public rendergraph::NodeInterface<rendergraph::BaseOpacityNode> {
};
