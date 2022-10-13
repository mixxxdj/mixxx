#pragma once

#include "waveform/renderers/qopengl/shaderloader.h"
#include "waveform/renderers/qopengl/waveformrenderer.h"

namespace qopengl {
class WaveformShaderRenderer;
}

class qopengl::WaveformShaderRenderer : public qopengl::WaveformRenderer,
                                        public qopengl::ShaderLoader {
  public:
    explicit WaveformShaderRenderer(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~WaveformShaderRenderer() override;
};
