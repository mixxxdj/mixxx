#pragma once

#include <QSGNode>

#include "rendergraph/nodebase.h"

namespace rendergraph {
class Node;
class NodeBase;
} // namespace rendergraph

class rendergraph::Node : public QSGNode, public rendergraph::NodeBase {
  public:
    Node();
    ~Node();
};
