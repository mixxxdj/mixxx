#include "widget/winitialglwidget.h"

WInitialGLWidget::WInitialGLWidget(QWidget* parent)
        : WGLWidget(parent) {
}

void WInitialGLWidget::initializeGL() {
    emit onInitialized();
}
