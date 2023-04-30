#include "widget/wglwaveformwidget.h"
#ifdef MIXXX_USE_QOPENGL
#include <QApplication>
#include <QEvent>
#endif

WGLWaveformWidget::WGLWaveformWidget(QWidget* parent)
        : WGLWidget(parent) {
}

#ifdef MIXXX_USE_QOPENGL
bool WGLWaveformWidget::event(QEvent* event) {
    const auto t = event->type();
    // events to be forwarded from OpenGLWindow to the parent WWaveformViewer
    if (t == QEvent::MouseButtonPress || t == QEvent::MouseButtonRelease ||
            t == QEvent::MouseMove || t == QEvent::Wheel ||
            t == QEvent::DragEnter || t == QEvent::DragLeave ||
            t == QEvent::DragMove || t == QEvent::Drop || t == QEvent::Leave) {
        QApplication::sendEvent(parent(), event);
    }
    return WGLWidget::event(event);
}
#endif
