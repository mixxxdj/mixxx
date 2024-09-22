#pragma once

#include "backend/baseopacitynode.h"
#include "rendergraph/treenode.h"

namespace rendergraph {
class OpacityNode;
} // namespace rendergraph

class rendergraph::OpacityNode : public rendergraph::BaseOpacityNode,
                                 public rendergraph::TreeNode {
  public:
    OpacityNode();
};
