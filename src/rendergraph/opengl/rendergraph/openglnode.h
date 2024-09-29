#pragma once

#include "backend/baseopenglnode.h"
#include "rendergraph/treenode.h"

namespace rendergraph {
class OpenGLNode;
} // namespace rendergraph

class rendergraph::OpenGLNode : public rendergraph::BaseOpenGLNode,
                                public rendergraph::TreeNode {
  public:
    OpenGLNode();

    virtual void initializeGL() {
    }
    virtual void paintGL() {
    }
    virtual void resizeGL(int, int) {
    }
};
