#pragma once

#include "waveform/renderers/qopengl/iwaveformrenderer.h"
#include "waveform/renderers/qopengl/shaderloader.h"
#include "waveform/renderers/waveformrenderersignalbase.h"

class WaveformWidgetRenderer;

namespace qopengl {
class WaveformRendererSignalBase;
} // namespace qopengl

class qopengl::WaveformRendererSignalBase : public ::WaveformRendererSignalBase,
                                            public qopengl::IWaveformRenderer,
                                            public qopengl::ShaderLoader {
  public:
    explicit WaveformRendererSignalBase(WaveformWidgetRenderer* waveformWidget);
    ~WaveformRendererSignalBase() override = default;

    void draw(QPainter* painter, QPaintEvent* event) override {
    }

    IWaveformRenderer* qopenglWaveformRenderer() override {
        return this;
    }

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererSignalBase);
};
