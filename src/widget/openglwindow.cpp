#include "widget/openglwindow.h"

#include <QApplication>
#include <QResizeEvent>

#include "widget/tooltipqopengl.h"
#include "widget/wglwidget.h"

OpenGLWindow::OpenGLWindow(WGLWidget* widget)
        : m_pWidget(widget) {
    QSurfaceFormat format;
    format.setVersion(2, 1);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);
}

OpenGLWindow::~OpenGLWindow() {
}

void OpenGLWindow::initializeGL() {
    if (m_pWidget) {
        m_pWidget->initializeGL();
    }
}

void OpenGLWindow::paintGL() {
    if (m_pWidget && isExposed()) {
        if (m_dirty) {
            // Extra render and swap to avoid flickering when resizing
            m_pWidget->paintGL();
            m_pWidget->swapBuffers();
            m_dirty = false;
        }
        m_pWidget->paintGL();
    }
}

void OpenGLWindow::resizeGL(int w, int h) {
    if (m_pWidget) {
        m_pWidget->resizeGL(w, h);
        m_dirty = true;
    }
}

void OpenGLWindow::widgetDestroyed() {
    m_pWidget = nullptr;
}

bool OpenGLWindow::event(QEvent* ev) {
    bool result = QOpenGLWindow::event(ev);

    if (m_pWidget) {
        const auto t = ev->type();

        // Tooltip don't work by forwarding the events. This mimics the
        // tooltip behavior.
        if (ev->type() == QEvent::MouseMove) {
            ToolTipQOpenGL::singleton().start(
                    m_pWidget, dynamic_cast<QMouseEvent*>(ev)->globalPos());
        }
        if (ev->type() == QEvent::Leave) {
            ToolTipQOpenGL::singleton().stop(m_pWidget);
        }

        if (t == QEvent::Expose) {
            // This event is only for windows, so we need a method to inform
            // the widget
            m_pWidget->windowExposed();
        } else if (ev->type() != QEvent::Resize && ev->type() != QEvent::Move) {
            // Forward event to m_pWidget, except for resize and move events.
            // The widget was the first to receive the geometry changed passed
            // them to the OpenGLWindow. If we forward the events back to the
            // m_pWidget we quickly overflow the event queue.

            QApplication::sendEvent(m_pWidget, ev);
        }
    }

    return result;
}
