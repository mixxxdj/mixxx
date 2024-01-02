#pragma once

#include "shaders/rgbshader.h"
#include "util/class.h"
#include "waveform/renderers/allshader/vertexbuffer.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

namespace allshader {
class WaveformRendererLRRGB;
}

class allshader::WaveformRendererLRRGB final : public allshader::WaveformRendererSignalBase {
  public:
    explicit WaveformRendererLRRGB(WaveformWidgetRenderer* waveformWidget);

    // override ::WaveformRendererSignalBase
    void onSetup(const QDomNode& node) override;

    void initializeGL() override;
    void paintGL() override;

  private:
    mixxx::RGBShader m_shader;
    Vector2DRGBVertexBuffer m_vertices;

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererLRRGB);
};
