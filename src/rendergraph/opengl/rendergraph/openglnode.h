#pragma once

#include "backend/openglnode.h"
#include "rendergraph/basenode.h"

namespace rendergraph {
class OpenGLNode;
} // namespace rendergraph

class rendergraph::OpenGLNode : public rendergraph::backend::OpenGLNode,
                                public rendergraph::BaseNode {
  public:
    OpenGLNode();

    virtual void initializeGL() {
    }
    virtual void paintGL() {
    }
    virtual void resizeGL(int, int) {
    }
};
