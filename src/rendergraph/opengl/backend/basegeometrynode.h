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
    void initializeBackend() override;
    void renderBackend() override;
};
