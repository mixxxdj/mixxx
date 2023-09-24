#pragma once

#include "waveform/renderers/allshader/waveformrendererabstract.h"
#include "waveform/renderers/waveformrendererabstract.h"

class WaveformWidgetRenderer;

namespace allshader {
class WaveformRenderer;
}

class allshader::WaveformRenderer : public ::WaveformRendererAbstract,
                                    public allshader::WaveformRendererAbstract {
  public:
    explicit WaveformRenderer(WaveformWidgetRenderer* widget);

    // Pure virtual from allshader::WaveformRendererAbstract.
    // Renderers that use QPainter functionality implement this.
    // But as all classes derived from allshader::WaveformRenderer
    // will only use openGL functions (combining QPainter and
    // QOpenGLWindow has bad performance), we leave this empty.
    // Should never be called.
    void draw(QPainter* painter, QPaintEvent* event) override final;

    allshader::WaveformRendererAbstract* allshaderWaveformRenderer() override final {
        // This class is indirectly derived from
        // WaveformWidgetRenderer, which has a member
        // QList<allshader::WaveformRendererAbstract*> m_rendererStack;
        // In the case of allshader::WaveformRenderer widgets,
        // all the items on this stack are derived from
        // allshader::WaveformRendererAbstract and we use this method to
        // access them as such. (We could also have used a
        // dynamic cast (or even static cast instead)
        return this;
    }
};
