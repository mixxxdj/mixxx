#pragma once

#include <QGLWidget>

#include "waveform/renderers/glwaveformrenderer.h"
#include "waveform/sharedglcontext.h"
#include "waveform/widgets/waveformwidgetabstract.h"

QT_FORWARD_DECLARE_CLASS(QString)

/// GLWaveformWidgetAbstract is a WaveformWidgetAbstract & QGLWidget. Its optional
/// member GLWaveformRenderer* m_pGlRenderer can implement a virtual method
/// onInitializeGL, which will be called from GLWaveformRenderer::initializeGL
/// (which overrides QGLWidget::initializeGL). This can be used for initialization
/// that must be deferred until the GL context has been initialized and that can't
/// be done in the constructor.
class GLWaveformWidgetAbstract : public WaveformWidgetAbstract, public QGLWidget {
  public:
    GLWaveformWidgetAbstract(const QString& group, QWidget* parent)
            : WaveformWidgetAbstract(group),
              QGLWidget(parent, SharedGLContext::getWidget())
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
              ,
              m_pGlRenderer(nullptr)
#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
    {
    }

  protected:
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
    void initializeGL() override {
        if (m_pGlRenderer) {
            m_pGlRenderer->onInitializeGL();
        }
    }

    GLWaveformRenderer* m_pGlRenderer;
#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
};
