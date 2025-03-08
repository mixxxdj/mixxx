#pragma once

#include "backend/basenode.h"

namespace rendergraph {
class BaseOpacityNode;
} // namespace rendergraph

class rendergraph::BaseOpacityNode : public rendergraph::BaseNode {
  public:
    BaseOpacityNode() = default;
    virtual ~BaseOpacityNode() = default;

    void setOpacity(float opacity) {
        m_opacity = opacity;
    }
    bool isSubtreeBlocked() const override {
        return m_opacity == 0.f;
    }

  private:
    float m_opacity{1.f};
};
