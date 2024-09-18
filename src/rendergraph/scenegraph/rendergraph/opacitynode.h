#pragma once

#include <QSGOpacityNode>

#include "rendergraph/node.h"

namespace rendergraph {
class OpacityNode;
} // namespace rendergraph

class rendergraph::OpacityNode : public QSGOpacityNode, public rendergraph::NodeBase {
  public:
    OpacityNode();
    ~OpacityNode();
};
