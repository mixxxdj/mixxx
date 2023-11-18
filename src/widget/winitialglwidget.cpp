#include "widget/winitialglwidget.h"

#include <QOpenGLFunctions>

#include "moc_winitialglwidget.cpp"

WInitialGLWidget::WInitialGLWidget(QWidget* pParent)
        : WGLWidget(pParent) {
}

void WInitialGLWidget::paintGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void WInitialGLWidget::initializeGL() {
    emit onInitialized();
}
