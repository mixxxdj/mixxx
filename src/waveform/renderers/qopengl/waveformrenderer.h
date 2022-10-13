#pragma once

#include <QOpenGLFunctions>

#include "waveform/renderers/qopengl/iwaveformrenderer.h"
#include "waveform/renderers/waveformrendererabstract.h"

class WaveformWidgetRenderer;

namespace qopengl {
class WaveformRenderer;
} // namespace qopengl

class qopengl::WaveformRenderer : public WaveformRendererAbstract, public IWaveformRenderer {
  public:
    explicit WaveformRenderer(WaveformWidgetRenderer* widget);
    ~WaveformRenderer();

    void draw(QPainter* painter, QPaintEvent* event) override {
    }

    IWaveformRenderer* qopenglWaveformRenderer() override {
        return this;
    }
};
