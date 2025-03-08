#pragma once

#include <QQuickWindow>
#include <gsl/pointers>

namespace rendergraph {
class Context;
}

class rendergraph::Context {
  public:
    Context(gsl::not_null<QQuickWindow*> pWindow);
    gsl::not_null<QQuickWindow*> window() const;

  private:
    QQuickWindow* m_pWindow;
};
