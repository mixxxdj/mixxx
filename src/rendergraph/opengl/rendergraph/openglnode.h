#pragma once

#include <QOpenGLFunctions>

#include "rendergraph/node.h"

namespace rendergraph {
class OpenGLNode;
} // namespace rendergraph

class rendergraph::OpenGLNode : public rendergraph::Node, public QOpenGLFunctions {
  public:
    OpenGLNode();
    ~OpenGLNode();

    virtual void initializeGL() {
    }
    virtual void paintGL() {
    }
    virtual void resizeGL(int, int) {
    }
    void initialize() override;
    void render() override;
    void resize(int w, int h) override;
};
