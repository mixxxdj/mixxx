#pragma once

#include "waveform/renderers/deprecated/glwaveformrenderer.h"
#include "waveform/renderers/waveformrenderersignalbase.h"

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

/// GLWaveformRendererSignal is a WaveformRendererAbstract which directly calls OpenGL functions.
///
/// Note that the Qt OpenGL WaveformRendererAbstracts are not GLWaveformRenderers because
/// they do not call OpenGL functions directly. Instead, they inherit QGLWidget and use the
/// QPainter API which Qt translates to OpenGL under the hood.
class GLWaveformRendererSignal : public WaveformRendererSignalBase, public GLWaveformRenderer {
  public:
    GLWaveformRendererSignal(WaveformWidgetRenderer* waveformWidgetRenderer,
            ::WaveformRendererSignalBase::Options options)
            : WaveformRendererSignalBase(waveformWidgetRenderer, options) {
    }
};

#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
