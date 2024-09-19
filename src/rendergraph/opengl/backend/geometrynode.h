#pragma once

#include <QOpenGLFunctions>

#include "backend/node.h"

namespace rendergraph::backend {
class GeometryNode;
}

class rendergraph::backend::GeometryNode : public rendergraph::backend::Node,
                                           public QOpenGLFunctions {
  protected:
    GeometryNode() = default;

  public:
    void initializeBackend() override;
    void renderBackend() override;
};
