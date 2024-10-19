#pragma once

#include <QOpenGLFunctions>

#include "backend/basenode.h"

namespace rendergraph {
class BaseGeometryNode;
}

class rendergraph::BaseGeometryNode : public rendergraph::BaseNode,
                                      public QOpenGLFunctions {
  protected:
    BaseGeometryNode() = default;

  public:
    void initialize() override;
    void render() override;
    void resize(int w, int h) override;
};
