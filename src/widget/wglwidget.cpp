#include "widget/wglwidget.h"

WGLWidget::WGLWidget(QWidget* parent, WGLWidget* shareWidget)
        : QWidget(parent) {
    if (shareWidget) {
        m_pOpenGLWindow = new QOpenGLWindow(shareWidget->m_pOpenGLWindow->context());
    } else {
        m_pOpenGLWindow = new QOpenGLWindow();
    }
    createWindowContainer(m_pOpenGLWindow, this);
}
