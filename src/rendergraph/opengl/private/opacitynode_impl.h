#pragma once

#include <QOpenGLFunctions>

#include "material_impl.h"
#include "node_impl.h"
#include "rendergraph/opacitynode.h"

// TODO implement opacity of subnodes.
// For now we only use this for visibility,
// mimicking QSGOpacityNode isSubtreeBlocked()

class rendergraph::OpacityNode::Impl : public rendergraph::NodeImplBase {
  public:
    Impl(OpacityNode* pOwner)
            : NodeImplBase(pOwner) {
    }
    void setOpacity(float opacity) {
        m_opacity = opacity;
    }

    bool isSubtreeBlocked() const override {
        return m_opacity == 0.f || NodeImplBase::isSubtreeBlocked();
    }

  private:
    float m_opacity{1.f};
};
