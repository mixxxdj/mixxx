#include "rendergraph/context.h"

using namespace rendergraph;

Context::Context(gsl::not_null<QQuickWindow*> pWindow)
        : m_pWindow(pWindow) {
}

gsl::not_null<QQuickWindow*> Context::window() const {
    return m_pWindow;
}
