#pragma once

#include "rendergraph/node.h"

namespace rendergraph {
class OpacityNode;
} // namespace rendergraph

class rendergraph::OpacityNode : public rendergraph::Node {
  public:
    OpacityNode();
    ~OpacityNode();
    void setOpacity(float opacity);
    bool isSubtreeBlocked() const override;

  private:
    float m_opacity{1.f};
};
