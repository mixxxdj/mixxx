#pragma once

#ifndef MIXXX_USE_QOPENGL
#include <QGLContext>
#endif
#include <QOpenGLFunctions_2_1>

#include "waveform/renderers/waveformrenderersignalbase.h"

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

/// GLWaveformRenderer is a WaveformRendererAbstract which directly calls OpenGL functions.
///
/// Note that the Qt OpenGL WaveformRendererAbstracts are not GLWaveformRenderers because
/// they do not call OpenGL functions directly. Instead, they inherit QGLWidget and use the
/// QPainter API which Qt translates to OpenGL under the hood.
class GLWaveformRenderer : public WaveformRendererSignalBase, protected QOpenGLFunctions_2_1 {
  public:
    GLWaveformRenderer(WaveformWidgetRenderer* waveformWidgetRenderer)
            : WaveformRendererSignalBase(waveformWidgetRenderer)
#ifndef MIXXX_USE_QOPENGL
              ,
              m_pLastContext(nullptr)
#endif
    {
    }

    virtual void onInitializeGL() {
        initializeOpenGLFunctions();
    }

  protected:
    // Somehow QGLWidget does not call QGLWidget::initializeGL on macOS, so hack around that
    // by calling this in `draw` when the QGLContext has been made current.
    // TODO: remove this when upgrading to QOpenGLWidget
    void maybeInitializeGL() {
#ifndef MIXXX_USE_QOPENGL
        if (QGLContext::currentContext() != m_pLastContext) {
            onInitializeGL();
            m_pLastContext = QGLContext::currentContext();
        }
#endif
    }

  private:
#ifndef MIXXX_USE_QOPENGL
    const QGLContext* m_pLastContext;
#endif
};

#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
