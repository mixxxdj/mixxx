#include "widget/wglwidget.h"

#ifdef MIXXX_USE_QGLWIDGET

#include "waveform/sharedglcontext.h"

WGLWidget::WGLWidget(QWidget* parent)
        : QGLWidget(parent, SharedGLContext::getWidget()) {
}

bool WGLWidget::isContextValid() const {
    return context()->isValid();
}

bool WGLWidget::isContextSharing() const {
    return context()->isSharing();
}

void WGLWidget::makeCurrentIfNeeded() {
    if (QGLContext::currentContext() != context()) {
        makeCurrent();
    }
}

#else

#include <QDebug>
#include <QOpenGLWindow>
#include <QResizeEvent>

OpenGLWindow::OpenGLWindow(WGLWidget* widget)
        : m_pWidget(widget) {
#ifndef MIXXX_USE_VSYNCTHREAD
    connect(this, &QOpenGLWindow::frameSwapped, this, &OpenGLWindow::onFrameSwapped);
#endif
}

void OpenGLWindow::initializeGL() {
    if (m_pWidget) {
        m_pWidget->initializeGL();
    }
}

void OpenGLWindow::paintGL() {
    if (m_pWidget) {
        m_pWidget->renderGL(this);
    }
}

void OpenGLWindow::resizeGL(int w, int h) {
}

void OpenGLWindow::clearWidget() {
    m_pWidget = nullptr;
}

bool OpenGLWindow::event(QEvent* ev) {
    bool result = QOpenGLWindow::event(ev);

    if (m_pWidget) {
        const auto t = ev->type();
        if (t == QEvent::Expose) {
            m_pWidget->resetVisualState();
        }

        if (t == QEvent::MouseButtonDblClick || t == QEvent::MouseButtonPress ||
                t == QEvent::MouseButtonRelease || t == QEvent::MouseMove ||
                t == QEvent::DragEnter || t == QEvent::DragLeave ||
                t == QEvent::DragMove || t == QEvent::Drop) {
            m_pWidget->handleEventFromWindow(ev);
        }
    }

    return result;
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
    if (m_pWidget) {
        m_pWidget->preRenderGL(this);
    }
    update();
}

WGLWidget::WGLWidget(QWidget* parent)
        : QWidget(parent), m_pOpenGLWindow(new OpenGLWindow(this)) {
    QSurfaceFormat format;
    format.setSwapInterval(1);
    m_pOpenGLWindow->setFormat(format);
    m_pContainerWidget = createWindowContainer(m_pOpenGLWindow, this);
}

WGLWidget::~WGLWidget() {
    m_pOpenGLWindow->clearWidget();
}

bool WGLWidget::isValid() const {
    return m_pOpenGLWindow->isValid();
}

void WGLWidget::resizeEvent(QResizeEvent* event) {
    const auto size = event->size();
    m_pContainerWidget->resize(size);
}

void WGLWidget::handleEventFromWindow(QEvent* e) {
    event(e);
}

void WGLWidget::renderGL(OpenGLWindow* window) {
    // show red to make clear this should be implemented by derived class
    glClearColor(1.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void WGLWidget::preRenderGL(OpenGLWindow* window) {
}

bool WGLWidget::isContextValid() const {
    return true;
}

bool WGLWidget::isContextSharing() const {
    return true;
}

void WGLWidget::setAutoBufferSwap(bool) {
}

void WGLWidget::makeCurrentIfNeeded() {
    if (m_pOpenGLWindow->context() != QOpenGLContext::currentContext()) {
        m_pOpenGLWindow->makeCurrent();
    }
}

void WGLWidget::initializeGL() {
    // to be implemented in derived widgets if needed
}

void WGLWidget::resetVisualState() {
    // rename this
}

void WGLWidget::swapBuffers() {
    // not used when driven by WOpenGLWindow::frameSwapped, but here to be able to compile the vsyncthread driver code that uses WGLWidget derived from QGLWidget
    // and when driving this from the vsyncthread
    makeCurrentIfNeeded();
    if (m_pOpenGLWindow->context()) {
        m_pOpenGLWindow->context()->swapBuffers(m_pOpenGLWindow->context()->surface());
    }
}

bool WGLWidget::shouldRender() {
    // TODO m0dB more things to check here?
    return isVisible() && m_pOpenGLWindow->isValid();
}

QOpenGLContext* WGLWidget::context() const {
    return m_pOpenGLWindow->context();
}

QOpenGLWindow* WGLWidget::window() const {
    return m_pOpenGLWindow;
}

#endif
