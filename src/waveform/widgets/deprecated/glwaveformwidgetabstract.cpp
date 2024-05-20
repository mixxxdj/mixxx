#include "waveform/widgets/deprecated/glwaveformwidgetabstract.h"

#include <QApplication>
#include <QWheelEvent>

GLWaveformWidgetAbstract::GLWaveformWidgetAbstract(const QString& group, QWidget* parent)
        : WaveformWidgetAbstract(group),
          WGLWidget(parent) {
}

void GLWaveformWidgetAbstract::wheelEvent(QWheelEvent* event) {
    QApplication::sendEvent(parentWidget(), event);
    event->accept();
}
