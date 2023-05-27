#include "widget/winitialglwidget.h"

#include <QOpenGLFunctions>

WInitialGLWidget::WInitialGLWidget(QWidget* parent)
        : WGLWidget(parent) {
}

void WInitialGLWidget::initializeGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    emit onInitialized();
}
