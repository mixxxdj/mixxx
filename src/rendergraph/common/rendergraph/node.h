#pragma once

#include "backend/node.h"
#include "rendergraph/basenode.h"

namespace rendergraph {
class Node;
} // namespace rendergraph

class rendergraph::Node : public rendergraph::backend::Node, public rendergraph::BaseNode {
  public:
    Node();
};
