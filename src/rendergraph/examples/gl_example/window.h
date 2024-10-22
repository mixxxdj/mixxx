#pragma once

#include <QOpenGLWindow>
#include <memory>

#include "rendergraph/engine.h"

namespace rendergraph {
class Graph;
}

class Window : public QOpenGLWindow {
  public:
    Window();

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void closeEvent(QCloseEvent* ev) override;

  private:
    std::unique_ptr<rendergraph::Engine> m_pEngine;
};
