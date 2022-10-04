#include "widget/wglwidget.h"

#include <QDebug>
#include <QOpenGLWindow>
#include <QResizeEvent>

OpenGLWindow::OpenGLWindow(WGLWidget* widget)
        : m_pWidget(widget) {
    connect(this, &QOpenGLWindow::frameSwapped, this, &OpenGLWindow::onFrameSwapped);
}

void OpenGLWindow::initializeGL() {
    m_pWidget->initializeGL();
}

void OpenGLWindow::paintGL() {
    m_pWidget->renderGL(this);
}

void OpenGLWindow::resizeGL(int w, int h) {
}

PerformanceTimer& OpenGLWindow::getMyTimer() const {
    static PerformanceTimer s_timer;
    return s_timer;
}

const PerformanceTimer& OpenGLWindow::getTimer() const {
    return getMyTimer();
}

int OpenGLWindow::getMicrosUntilSwap() const {
    return 1000000 / 60;
}

void OpenGLWindow::onFrameSwapped() {
    if (getMyTimer().elapsed().toDoubleMicros() > getMicrosUntilSwap() * 0.5) {
        getMyTimer().restart();
    }
    m_pWidget->preRenderGL(this);
    update();
}
#include <iostream>
WGLWidget::WGLWidget(QWidget* parent)
        : QWidget(parent) {
    m_pOpenGLWindow = new OpenGLWindow(this);
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3, 3);
    format.setSwapInterval(1);
    m_pContainerWidget = createWindowContainer(m_pOpenGLWindow, this);
}

bool WGLWidget::isValid() const {
    return true;
}

void WGLWidget::resizeEvent(QResizeEvent* event) {
    const auto size = event->size();
    m_pContainerWidget->resize(size);
}

void WGLWidget::renderGL(OpenGLWindow* window) {
    // show red to make clear this should be implemented by derived class
    glClearColor(1.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void WGLWidget::preRenderGL(OpenGLWindow* window) {
}

QOpenGLContext* WGLWidget::context() const {
    return m_pOpenGLWindow->context();
}

void WGLWidget::makeCurrent() {
    m_pOpenGLWindow->makeCurrent();
}
