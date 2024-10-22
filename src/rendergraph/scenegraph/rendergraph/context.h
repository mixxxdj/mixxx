#pragma once

#include <QQuickWindow>

namespace rendergraph {
class Context;
}

class rendergraph::Context {
  public:
    Context(QQuickWindow* pWindow);
    QQuickWindow* window() const;

  private:
    QQuickWindow* m_pWindow;
};
