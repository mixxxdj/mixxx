#pragma once

#include <QOpenGLFunctions>

#include "rendergraph/node.h"

namespace rendergraph {
class OpenGLNode;
} // namespace rendergraph

class rendergraph::OpenGLNode : public rendergraph::Node, public QOpenGLFunctions {
  public:
    class Impl;

    OpenGLNode();
    ~OpenGLNode();

    virtual void initializeGL() {
    }
    virtual void paintGL() {
    }
    virtual void resizeGL(int, int) {
    }

  private:
    OpenGLNode(NodeImplBase* pImpl);
};
