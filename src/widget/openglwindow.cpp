#include "widget/openglwindow.h"

#include <QResizeEvent>

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
        m_pWidget->swapBuffers();
        m_pWidget->renderGL();
    }
}

void OpenGLWindow::resizeGL(int w, int h) {
    if (m_pWidget) {
        m_pWidget->resizeGL(w, h);
    }
}

void OpenGLWindow::widgetDestroyed() {
    m_pWidget = nullptr;
}

bool OpenGLWindow::event(QEvent* ev) {
    bool result = QOpenGLWindow::event(ev);

    if (m_pWidget) {
        const auto t = ev->type();
        // Forward the following events to the WGLWidget
        if (t == QEvent::MouseButtonDblClick || t == QEvent::MouseButtonPress ||
                t == QEvent::MouseButtonRelease || t == QEvent::MouseMove ||
                t == QEvent::Enter || t == QEvent::Leave ||
                t == QEvent::DragEnter || t == QEvent::DragLeave ||
                t == QEvent::DragMove || t == QEvent::Drop || t == QEvent::Wheel) {
            m_pWidget->handleEventFromWindow(ev);
        }
        if (t == QEvent::Expose) {
            m_pWidget->windowExposed();
        }
    }

    return result;
}
