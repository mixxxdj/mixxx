#pragma once

#include "waveform/renderers/waveformrendererabstract.h"

class WaveformWidgetRenderer;

namespace allshader {
class WaveformRenderer;
} // namespace allshader

class allshader::WaveformRenderer : public ::WaveformRendererAbstract {
  public:
    explicit WaveformRenderer(WaveformWidgetRenderer* widget);

    // Pure virtual from allshader::WaveformRendererAbstract.
    // Renderers that use QPainter functionality implement this.
    // But as all classes derived from allshader::WaveformRenderer
    // will only use openGL functions (combining QPainter and
    // QOpenGLWindow has bad performance), we leave this empty.
    // Should never be called.
    void draw(QPainter* painter, QPaintEvent* event) override final;
};
