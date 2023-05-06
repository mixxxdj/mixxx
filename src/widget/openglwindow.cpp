#include "widget/openglwindow.h"

#include <QApplication>
#include <QResizeEvent>

#include "widget/tooltipqopengl.h"
#include "widget/trackdroptarget.h"
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
    const auto t = ev->type();

    bool result = QOpenGLWindow::event(ev);

    if (m_pWidget) {
        // Tooltip don't work by forwarding the events. This mimics the
        // tooltip behavior.
        if (t == QEvent::MouseMove) {
            ToolTipQOpenGL::singleton().start(
                    m_pWidget, dynamic_cast<QMouseEvent*>(ev)->globalPos());
        }
        if (t == QEvent::Leave) {
            ToolTipQOpenGL::singleton().stop(m_pWidget);
        }

        if (t == QEvent::DragEnter || t == QEvent::DragMove ||
                t == QEvent::DragLeave || t == QEvent::Drop) {
            // Drag & Drop events are not delivered correctly when using QApplication::sendEvent
            // and even result in a recursive call to this method, so we use our own mechanism.
            if (m_pWidget->trackDropTarget()) {
                return m_pWidget->trackDropTarget()->handleDragAndDropEventFromWindow(ev);
            }

            ev->ignore();
            return false;
        }

        if (t == QEvent::Resize || t == QEvent::Move || t == QEvent::Expose) {
            // - Resize and Move
            //    Any change to the geometry comes from m_pWidget and its child m_pContainerWidget.
            //    If we send the resulting events back to the m_pWidget we will quickly overflow
            //    the event queue with repeated resize and move events.
            // - Expose
            //    This event is only for windows
            return result;
        }

        // Send all remaining events to the widget that owns the window
        // container widget that contains this QOpenGLWindow. With this mouse
        // events, keyboard events, etc all arrive as intended, including the
        // events for the WWaveformViewer that contains the waveform widget.
        QApplication::sendEvent(m_pWidget, ev);
    }

    return result;
}
