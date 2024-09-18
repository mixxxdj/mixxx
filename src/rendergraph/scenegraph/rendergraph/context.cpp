#include "rendergraph/context.h"

using namespace rendergraph;

Context::Context() = default;

Context::~Context() = default;

void Context::setWindow(QQuickWindow* pWindow) {
    m_pWindow = pWindow;
}

QQuickWindow* Context::window() const {
    return m_pWindow;
}
