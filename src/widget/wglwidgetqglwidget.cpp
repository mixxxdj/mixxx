#include <QWindow>

#include "waveform/sharedglcontext.h"
#include "widget/wglwidget.h"

WGLWidget::WGLWidget(QWidget* parent)
        : QGLWidget(parent, SharedGLContext::getWidget()) {
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAutoFillBackground(false);
    setAutoBufferSwap(false);
    // Not interested in repaint or update calls, as we draw from the vsync thread
    setUpdatesEnabled(false);
}

bool WGLWidget::isContextValid() const {
    // A QGLWidget should always have a context, but it is possible that
    // the context is not valid. for example, if the underlying hardware
    // does not support the format attributes that were requested.
    return context()->isValid();
}

bool WGLWidget::isContextSharing() const {
    return context()->isSharing();
}

bool WGLWidget::shouldRender() const {
    return isValid() && isVisible() && windowHandle() && windowHandle()->isExposed();
}

void WGLWidget::makeCurrentIfNeeded() {
    if (context() != QGLContext::currentContext()) {
        makeCurrent();
    }
}
