#include <QResizeEvent>

#include "widget/openglwindow.h"
#include "widget/tooltipqopengl.h"
#include "widget/wglwidget.h"

WGLWidget::WGLWidget(QWidget* parent)
        : QWidget(parent) {
    // When the widget is resized or moved, the QOpenGLWindow visibly resizes
    // or moves before the widgets do. This can be solved by calling
    //   setAttribute(Qt::WA_PaintOnScreen);
    // here, but this comes with a clear performance penalty and drop in
    // frame rate.
}

WGLWidget::~WGLWidget() {
    ToolTipQOpenGL::singleton().stop(this);
    if (m_pOpenGLWindow) {
        m_pOpenGLWindow->widgetDestroyed();
    }
}

QPaintDevice* WGLWidget::paintDevice() {
    makeCurrentIfNeeded();
    return m_pOpenGLWindow;
}

void WGLWidget::setTrackDropTarget(TrackDropTarget* target)
{
    m_pTrackDropTarget = target;
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

bool WGLWidget::isContextSharing() const {
    return true;
}

void WGLWidget::makeCurrentIfNeeded() {
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
    // to be implemented in derived widgets if needed
}

void WGLWidget::swapBuffers() {
    if (shouldRender()) {
        m_pOpenGLWindow->context()->swapBuffers(m_pOpenGLWindow->context()->surface());
    }
}

bool WGLWidget::shouldRender() const {
    return m_pOpenGLWindow && m_pOpenGLWindow->isExposed();
}
