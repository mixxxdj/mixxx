#pragma once

#include "shaders/unicolorshader.h"
#include "util/class.h"
#include "waveform/renderers/allshader/vertexbuffer.h"
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
    Vector2DVertexBuffer m_vertices;

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererSimple);
};
