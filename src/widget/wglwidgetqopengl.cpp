#include <QResizeEvent>

#include "widget/openglwindow.h"
#include "widget/tooltipqopengl.h"
#include "widget/wglwidget.h"

WGLWidget::WGLWidget(QWidget* pParent)
        : QWidget(pParent),
          m_pOpenGLWindow(nullptr),
          m_pContainerWidget(nullptr),
          m_pTrackDropTarget(nullptr) {
    // When the widget is resized or moved, the QOpenGLWindow visibly resizes
    // or moves before the widgets do. This can be solved by calling
    //   setAttribute(Qt::WA_PaintOnScreen);
    // here, but this comes with a clear performance penalty and drop in
    // frame rate.
}

WGLWidget::~WGLWidget() {
    ToolTipQOpenGL::singleton().stop();
    if (m_pOpenGLWindow) {
        m_pOpenGLWindow->widgetDestroyed();
    }
}

QPaintDevice* WGLWidget::paintDevice() {
    makeCurrentIfNeeded();
    return m_pOpenGLWindow;
}

void WGLWidget::setTrackDropTarget(TrackDropTarget* pTarget) {
    m_pTrackDropTarget = pTarget;
}

TrackDropTarget* WGLWidget::trackDropTarget() const {
    return m_pTrackDropTarget;
}

void WGLWidget::showEvent(QShowEvent* event) {
    if (!m_pOpenGLWindow) {
        m_pOpenGLWindow = new OpenGLWindow(this);
        m_pContainerWidget = createWindowContainer(m_pOpenGLWindow, this);
        m_pContainerWidget->resize(size());
        m_pContainerWidget->show();
        m_pContainerWidget->setAutoFillBackground(true);
    }
    QWidget::showEvent(event);
}

void WGLWidget::resizeEvent(QResizeEvent* event) {
    if (m_pContainerWidget) {
        m_pContainerWidget->resize(event->size());
    }
    QWidget::resizeEvent(event);
}

bool WGLWidget::isContextValid() const {
    return m_pOpenGLWindow && m_pOpenGLWindow->context() && m_pOpenGLWindow->context()->isValid();
}

void WGLWidget::makeCurrentIfNeeded() {
    // On Android the Surface is destroyed on pause/rotation while the
    // QOpenGLWindow outlives it — makeCurrent() then spews "no EGLSurface"
    // and leaves a dead current context (18x in one session log). Skip.
    if (!shouldRender()) {
        return;
    }
    if (m_pOpenGLWindow && m_pOpenGLWindow->context() != QOpenGLContext::currentContext()) {
        m_pOpenGLWindow->makeCurrent();
    }
}

void WGLWidget::doneCurrent() {
    if (m_pOpenGLWindow) {
        m_pOpenGLWindow->doneCurrent();
    }
}

void WGLWidget::paintGL() {
    // to be implemented in derived widgets if needed
}

void WGLWidget::initializeGL() {
    // to be implemented in derived widgets if needed
}

void WGLWidget::resizeGL(int w, int h) {
    Q_UNUSED(w);
    Q_UNUSED(h);
    // to be implemented in derived widgets if needed
}

void WGLWidget::swapBuffers() {
    if (shouldRender()) {
        m_pOpenGLWindow->context()->swapBuffers(m_pOpenGLWindow->context()->surface());
    }
}

bool WGLWidget::shouldRender() const {
    if (!m_pOpenGLWindow) {
        return false;
    }

    if (!m_pOpenGLWindow->isValid()) {
        return false;
    }

    // Prefer isExposed() but fall back to isVisible() if the window
    // is not exposed but the widget is visible. This works around a Qt bug
    // where isExposed() incorrectly returns false during certain UI updates.
    // See: https://github.com/mixxxdj/mixxx/issues/15103
    // Fallback: if the widget itself is visible, render anyway — but only
    // when the GL context is actually valid. An exposed window with a dead
    // context (Android Surface destroyed) must not render: swapBuffers on
    // it fails with EGL_BAD_SURFACE and makeCurrent spews warnings.
    if (m_pOpenGLWindow->isExposed()) {
        auto* ctx = m_pOpenGLWindow->context();
        return ctx != nullptr && ctx->isValid() && ctx->surface() != nullptr;
    }
    // Fallback: if the widget itself is visible, render anyway
    if (isVisible()) {
        auto* ctx = m_pOpenGLWindow->context();
        return ctx != nullptr && ctx->isValid() && ctx->surface() != nullptr;
    }
    return false;
}

QOpenGLWindow* WGLWidget::getOpenGLWindow() const {
    return m_pOpenGLWindow;
}
