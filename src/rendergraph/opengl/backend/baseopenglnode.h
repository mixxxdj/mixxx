#pragma once

#include <QOpenGLFunctions>

#include "backend/basenode.h"

namespace rendergraph {
class BaseOpenGLNode;
} // namespace rendergraph

class rendergraph::BaseOpenGLNode : public rendergraph::BaseNode,
                                    public QOpenGLFunctions {
  public:
    BaseOpenGLNode() = default;
    virtual ~BaseOpenGLNode() = default;

    void initialize() override;
    void render() override;
    void resize(int w, int h) override;

    virtual void initializeGL() {
    }
    virtual void paintGL() {
    }
    virtual void resizeGL(int, int) {
    }
};
