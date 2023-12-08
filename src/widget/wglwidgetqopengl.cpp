#include <QResizeEvent>

#include "waveform/useframeswapped.h"
#include "waveform/waveformwidgetfactory.h"
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

        if (USE_FRAME_SWAPPED) {
            connect(m_pOpenGLWindow,
                    &QOpenGLWindow::frameSwapped,
                    this,
                    &WGLWidget::slotFrameSwapped,
                    Qt::DirectConnection);
        }
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
    assert(!USE_FRAME_SWAPPED);
    if (shouldRender()) {
        m_pOpenGLWindow->context()->swapBuffers(m_pOpenGLWindow->context()->surface());
    }
}

void WGLWidget::slotFrameSwapped() {
    // This frameSwapped is emitted after the potentially blocking buffer swap has
    // been done. Applications that wish to continuously repaint synchronized to
    // the vertical refresh, should issue an update() upon this signal. This allows
    // for a much smoother experience compared to the traditional usage of timers.
    m_pOpenGLWindow->update();
    WaveformWidgetFactory::instance()->frameSwapped();
}

bool WGLWidget::shouldRender() const {
    return m_pOpenGLWindow && m_pOpenGLWindow->isExposed();
}
