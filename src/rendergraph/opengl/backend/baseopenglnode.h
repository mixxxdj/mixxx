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
    void initializeBackend() override;
    void renderBackend() override;
    void resizeBackend(int w, int h) override;
};
