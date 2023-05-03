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
    // From here we call QApplication::sendEvent(m_pWidget, ev) to trigger
    // handling of the event as if it were received by the main window.
    // With drag move and drag leave events it may happen that this function
    // gets called recursive, potentially resulting in infinite recursion
    // and a stack overflow. The boolean m_handlingEvent protects against
    // this recursion.

    if (m_handlingEvent) {
        return false;
    }
    m_handlingEvent = true;

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
            // Any change to the geometry comes from m_pWidget and its child
            // m_pContainerWidget. If we send the resulting events back to the
            // m_pWidget we quickly overflow the event queue with repeated resize
            // and move events. All other events will be send to the widget.

            QApplication::sendEvent(m_pWidget, ev);
        }
    }

    m_handlingEvent = false;

    return result;
}
