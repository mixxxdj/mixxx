#pragma once

#include "backend/opacitynode.h"
#include "rendergraph/basenode.h"

namespace rendergraph {
class OpacityNode;
} // namespace rendergraph

class rendergraph::OpacityNode : public rendergraph::backend::OpacityNode,
                                 public rendergraph::BaseNode {
  public:
    OpacityNode();
};
