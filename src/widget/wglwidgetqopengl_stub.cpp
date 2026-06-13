// Stub implementations for WGLWidget on Android (QOPENGL=OFF)
// This file provides empty implementations for the virtual methods declared
// in wglwidgetqopengl.h so the linker can resolve symbols.
#include "widget/wglwidget.h"

WGLWidget::WGLWidget(QWidget* parent)
        : QWidget(parent),
          m_pOpenGLWindow(nullptr),
          m_pTrackDropTarget(nullptr) {
    Q_UNUSED(parent);
}

WGLWidget::~WGLWidget() {
}

bool WGLWidget::isContextValid() const {
    return false;
}

bool WGLWidget::shouldRender() const {
    return false;
}

void WGLWidget::makeCurrentIfNeeded() {
}

void WGLWidget::doneCurrent() {
}

void WGLWidget::swapBuffers() {
}

void WGLWidget::paintGL() {
}

void WGLWidget::resizeGL(int w, int h) {
    Q_UNUSED(w);
    Q_UNUSED(h);
}

void WGLWidget::initializeGL() {
}

void WGLWidget::setTrackDropTarget(TrackDropTarget* pTarget) {
    m_pTrackDropTarget = pTarget;
}

TrackDropTarget* WGLWidget::trackDropTarget() const {
    return m_pTrackDropTarget;
}

QOpenGLWindow* WGLWidget::getOpenGLWindow() const {
    return nullptr;
}

void WGLWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
}

void WGLWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
}

QPaintDevice* WGLWidget::paintDevice() {
    return nullptr;
}
