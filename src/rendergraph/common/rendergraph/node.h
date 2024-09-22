#pragma once

#include "backend/basenode.h"
#include "rendergraph/treenode.h"

namespace rendergraph {
class Node;
} // namespace rendergraph

class rendergraph::Node : public rendergraph::BaseNode, public rendergraph::TreeNode {
  public:
    Node();
};
