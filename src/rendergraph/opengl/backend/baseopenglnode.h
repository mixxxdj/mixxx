#pragma once

#include <QOpenGLFunctions>

#include "backend/basenode.h"

namespace rendergraph {
class BaseOpenGLNode;
}

class rendergraph::BaseOpenGLNode : public rendergraph::BaseNode,
                                    public QOpenGLFunctions {
  protected:
    BaseOpenGLNode() = default;

  public:
    void initialize() override;
    void render() override;
    void resize(int w, int h) override;
};
