#include "waveform/widgets/glwaveformwidgetabstract.h"

GLWaveformWidgetAbstract::GLWaveformWidgetAbstract(const QString& group, QWidget* parent)
        : WaveformWidgetAbstract(group),
          WGLWidget(parent) {
}
