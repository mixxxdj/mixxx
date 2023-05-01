#include "waveform/widgets/glwaveformwidgetabstract.h"

GLWaveformWidgetAbstract::GLWaveformWidgetAbstract(const QString& group, QWidget* parent)
        : WaveformWidgetAbstract(group),
          WGLWaveformWidget(parent)
{
}
