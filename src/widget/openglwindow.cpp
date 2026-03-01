#include "widget/openglwindow.h"

#include <QApplication>
#include <QResizeEvent>

#include "moc_openglwindow.cpp"
#include "waveform/waveformwidgetfactory.h"
#include "widget/tooltipqopengl.h"
#include "widget/trackdroptarget.h"
#include "widget/wglwidget.h"

OpenGLWindow::OpenGLWindow(WGLWidget* pWidget)
        : m_pWidget(pWidget),
          m_pTrackDropTarget(nullptr) {
    setFormat(WaveformWidgetFactory::getSurfaceFormat());
#ifdef __EMSCRIPTEN__
    // This is required to ensure that QOpenGLWindows have no minimum size (When
    // targeting WebAssembly, the widgets will otherwise always have a minimum
    // width and minimum height of 100 pixels).
    setFlag(Qt::FramelessWindowHint);
#endif
    // Prevent this window/widget from getting keyboard focus on click.
    setFlag(Qt::WindowDoesNotAcceptFocus);
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
        m_pWidget->paintGL();
    }
}

void OpenGLWindow::resizeGL(int w, int h) {
    if (m_pWidget) {
        // QGLWidget::resizeGL has a valid context (QOpenGLWindow::resizeGL does not), so we
        // mimic the same behaviour
        m_pWidget->makeCurrentIfNeeded();
        // QGLWidget::resizeGL has devicePixelRatio applied, so we mimic the same behaviour
        m_pWidget->resizeGL(static_cast<int>(static_cast<float>(w) * devicePixelRatio()),
                static_cast<int>(static_cast<float>(h) * devicePixelRatio()));
        // additional paint and swap to avoid flickering
        m_pWidget->paintGL();
        m_pWidget->swapBuffers();

        m_pWidget->doneCurrent();
    }
}

void OpenGLWindow::widgetDestroyed() {
    m_pWidget = nullptr;
}

bool OpenGLWindow::event(QEvent* pEv) {
    // From here we call QApplication::sendEvent(m_pWidget, ev) to trigger
    // handling of the event as if it were received by the main window.
    // With drag move and drag leave events it may happen that this function
    // gets called recursive, potentially resulting in infinite recursion
    // and a stack overflow. The boolean m_handlingEvent protects against
    // this recursion.
    const auto t = pEv->type();

    bool result = QOpenGLWindow::event(pEv);

    if (m_pWidget) {
        // Tooltip don't work by forwarding the events. This mimics the
        // tooltip behavior.
        if (t == QEvent::MouseMove) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QPoint eventPosition = dynamic_cast<QMouseEvent*>(pEv)->globalPosition().toPoint();
#else
            QPoint eventPosition = dynamic_cast<QMouseEvent*>(pEv)->globalPos();
#endif
            ToolTipQOpenGL::singleton().start(m_pWidget, eventPosition);
        }

        if (t == QEvent::Leave) {
            ToolTipQOpenGL::singleton().stop();
        }

        // Drag & Drop events are not delivered correctly when using QApplication::sendEvent
        // and even result in a recursive call to this method, so we use our own mechanism.

        if (t == QEvent::DragEnter) {
            DEBUG_ASSERT(!m_pTrackDropTarget);
            TrackDropTarget* pTrackDropTarget = m_pWidget->trackDropTarget();
            if (pTrackDropTarget) {
                bool ret = pTrackDropTarget->handleDragAndDropEventFromWindow(pEv);
                if (pEv->isAccepted()) {
                    m_pTrackDropTarget = pTrackDropTarget;
                }
                return ret;
            }
            pEv->ignore();
            return false; // clazy:exclude=base-class-event
        }

        if (t == QEvent::DragMove) {
            if (m_pTrackDropTarget) {
                bool ret = m_pTrackDropTarget->handleDragAndDropEventFromWindow(pEv);
                return ret;
            }
            pEv->ignore();
            return false; // clazy:exclude=base-class-event
        }

        if (t == QEvent::DragLeave || t == QEvent::Drop) {
            if (m_pTrackDropTarget) {
                bool ret = m_pTrackDropTarget->handleDragAndDropEventFromWindow(pEv);
                m_pTrackDropTarget = nullptr;
                return ret;
            }
            pEv->ignore();
            return false; // clazy:exclude=base-class-event
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
        QApplication::sendEvent(m_pWidget, pEv);
    }

    return result;
}
