#include "rendergraph/context.h"

using namespace rendergraph;

Context::Context(QQuickWindow* pWindow)
        : m_pWindow(pWindow) {
}

QQuickWindow* Context::window() const {
    return m_pWindow;
}
