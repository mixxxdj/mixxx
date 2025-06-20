#pragma once

#include "backend/basenode.h"
#include "rendergraph/nodeinterface.h"

namespace rendergraph {
class Node;
} // namespace rendergraph

class rendergraph::Node : public rendergraph::NodeInterface<rendergraph::BaseNode> {
};
