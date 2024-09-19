#pragma once

#include <QOpenGLFunctions>

#include "backend/node.h"

namespace rendergraph::backend {
class OpenGLNode;
}

class rendergraph::backend::OpenGLNode : public rendergraph::backend::Node,
                                         public QOpenGLFunctions {
  protected:
    OpenGLNode() = default;

  public:
    void initializeBackend() override;
    void renderBackend() override;
    void resizeBackend(int w, int h) override;
};
