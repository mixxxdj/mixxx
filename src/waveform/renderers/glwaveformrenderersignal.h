#pragma once

#include <QGLContext>
#include <QOpenGLFunctions_2_1>

#include "waveform/renderers/waveformrenderersignalbase.h"

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

/// GLWaveformRenderer is a WaveformRendererSignalBase which directly calls OpenGL functions.
///
/// Note that the Qt OpenGL WaveformRendererAbstracts are not GLWaveformRenderers because
/// they do not call OpenGL functions directly. Instead, they inherit QGLWidget and use the
/// QPainter API which Qt translates to OpenGL under the hood.
class GLWaveformRendererSignal : public WaveformRendererSignalBase, protected QOpenGLFunctions_2_1 {
  public:
    GLWaveformRendererSignal(WaveformWidgetRenderer* waveformWidgetRenderer)
            : WaveformRendererSignalBase(waveformWidgetRenderer),
              m_pLastContext(nullptr) {
    }

    virtual void onInitializeGL() {
        initializeOpenGLFunctions();
    }

  protected:
    // Somehow QGLWidget does not call QGLWidget::initializeGL on macOS, so hack around that
    // by calling this in `draw` when the QGLContext has been made current.
    // TODO: remove this when upgrading to QOpenGLWidget
    void maybeInitializeGL() {
        if (QGLContext::currentContext() != m_pLastContext) {
            onInitializeGL();
            m_pLastContext = QGLContext::currentContext();
        }
    }

  private:
    const QGLContext* m_pLastContext;
};

#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
