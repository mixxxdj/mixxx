#include "widget/openglwindow.h"

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

void OpenGLWindow::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLWindow::initializeGL() {
    if (m_pWidget) {
        m_pWidget->initializeGL();
    }
}

void OpenGLWindow::paintGL() {
    if (m_pWidget && isExposed()) {
        m_pWidget->renderGL();
    }
}

void OpenGLWindow::resizeGL(int w, int h) {
    if (m_pWidget) {
        m_pWidget->resizeGL(w, h);
        // To avoid flickering when resizing
        m_pWidget->makeCurrentIfNeeded();
        m_pWidget->renderGL();
        m_pWidget->swapBuffers();
        m_pWidget->doneCurrent();
    }
}

void OpenGLWindow::widgetDestroyed() {
    m_pWidget = nullptr;
}

bool OpenGLWindow::event(QEvent* ev) {
    bool result = QOpenGLWindow::event(ev);

    if (m_pWidget) {
        const auto t = ev->type();

        if (ev->type() == QEvent::MouseMove) {
            ToolTipQOpenGL::singleton().start(
                    m_pWidget, dynamic_cast<QMouseEvent*>(ev)->globalPos());
        }
        if (ev->type() == QEvent::Leave) {
            ToolTipQOpenGL::singleton().stop(m_pWidget);
        }

        m_pWidget->handleEventFromWindow(ev);

        if (t == QEvent::Expose) {
            m_pWidget->windowExposed();
        }
    }

    return result;
}
