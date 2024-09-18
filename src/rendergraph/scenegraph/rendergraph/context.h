#pragma once

class QQuickWindow;

namespace rendergraph {
class Context;
}

class rendergraph::Context {
  public:
    Context();
    ~Context();

    void setWindow(QQuickWindow* pWindow);
    QQuickWindow* window() const;

  private:
    QQuickWindow* m_pWindow;
};
