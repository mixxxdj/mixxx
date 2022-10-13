#include "waveform/widgets/glwaveformwidgetabstract.h"

#include "widget/wwaveformviewer.h"

GLWaveformWidgetAbstract::GLWaveformWidgetAbstract(const QString& group, QWidget* parent)
        : WaveformWidgetAbstract(group),
          WGLWidget(parent)
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
          ,
          m_pGlRenderer(nullptr)
#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
{
}

#ifdef MIXXX_USE_QOPENGL
// We need to forward events coming from the QOpenGLWindow
// (drag&drop, mouse) to the viewer
void GLWaveformWidgetAbstract::handleEventFromWindow(QEvent* ev) {
    auto viewer = dynamic_cast<WWaveformViewer*>(parent());
    if (viewer) {
        viewer->handleEventFromWindow(ev);
    }
}
#endif
