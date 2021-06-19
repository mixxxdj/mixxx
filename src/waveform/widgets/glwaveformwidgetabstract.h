#pragma once

#include <QGLWidget>
#include <QString>

#include "waveform/renderers/glwaveformrenderer.h"
#include "waveform/sharedglcontext.h"
#include "waveform/widgets/waveformwidgetabstract.h"

QT_FORWARD_DECLARE_CLASS(QString)

/// GLWaveformWidgetAbstract is a WaveformWidgetAbstract & QGLWidget that has
/// a GLWaveformRenderer member which requires initialization that must be
/// deferred until Qt calls QGLWidget::initializeGL and cannot be done in the
/// constructor.
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
