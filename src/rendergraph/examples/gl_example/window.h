#pragma once

#include <QOpenGLWindow>
#include <memory>

namespace rendergraph {
class Graph;
}

class Window : public QOpenGLWindow {
  public:
    Window();
    ~Window();

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void closeEvent(QCloseEvent* ev) override;

  private:
    std::unique_ptr<rendergraph::Graph> m_rendergraph;
};
