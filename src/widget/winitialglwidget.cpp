#include "widget/winitialglwidget.h"

WInitialGLWidget::WInitialGLWidget(QWidget* parent)
        : WGLWidget(parent) {
}

void WInitialGLWidget::windowExposed() {
    if (!m_windowExposedCalled) {
        m_windowExposedCalled = true;
        emit onWindowExposed();
    }
}
