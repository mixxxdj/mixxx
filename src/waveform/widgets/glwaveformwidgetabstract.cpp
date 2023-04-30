#include "waveform/widgets/glwaveformwidgetabstract.h"

GLWaveformWidgetAbstract::GLWaveformWidgetAbstract(const QString& group, QWidget* parent)
        : WaveformWidgetAbstract(group),
          WGLWaveformWidget(parent)
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
          ,
          m_pGlRenderer(nullptr)
#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
{
}
