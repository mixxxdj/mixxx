#pragma once

#include <QSGOpacityNode>

#include "rendergraph/node.h"

namespace rendergraph {
class OpacityNode;
} // namespace rendergraph

class rendergraph::OpacityNode : public QSGOpacityNode, public rendergraph::Node {
  public:
    OpacityNode();
    ~OpacityNode();
};
