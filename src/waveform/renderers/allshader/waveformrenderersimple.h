#pragma once

#include "shaders/unicolorshader.h"
#include "util/class.h"
#include "waveform/renderers/allshader/vertexdata.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

namespace allshader {
class WaveformRendererSimple;
}

class allshader::WaveformRendererSimple final : public allshader::WaveformRendererSignalBase {
  public:
    explicit WaveformRendererSimple(WaveformWidgetRenderer* waveformWidget);

    // override ::WaveformRendererSignalBase
    void onSetup(const QDomNode& node) override;

    void initializeGL() override;
    void paintGL() override;

  private:
    mixxx::UnicolorShader m_shader;
    VertexData m_vertices[2];

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererSimple);
};
