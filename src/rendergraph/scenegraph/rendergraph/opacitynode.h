#pragma once

#include "rendergraph/node.h"

namespace rendergraph {
class OpacityNode;
} // namespace rendergraph

class rendergraph::OpacityNode : public rendergraph::Node {
  public:
    class Impl;

    OpacityNode();
    ~OpacityNode();
    void setOpacity(float opacity);

  private:
    OpacityNode(NodeImplBase* pImpl);
};
