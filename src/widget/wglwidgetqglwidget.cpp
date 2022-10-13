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
    return context()->isValid();
}

bool WGLWidget::isContextSharing() const {
    return context()->isSharing();
}

bool WGLWidget::shouldRender() const {
    return true;
    return isValid() && isVisible() && windowHandle() && windowHandle()->isExposed();
}

void WGLWidget::makeCurrentIfNeeded() {
    // TODO m0dB is this really needed? is calling makeCurrent without this 'if' really a problem?
    // if (context() != QGLContext::currentContext()) {
    makeCurrent();
    //}
}
