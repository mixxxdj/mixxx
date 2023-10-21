#pragma once

#include "shaders/rgbshader.h"
#include "util/class.h"
#include "waveform/renderers/allshader/rgbdata.h"
#include "waveform/renderers/allshader/vertexdata.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

namespace allshader {
class WaveformRendererHSV;
}

class allshader::WaveformRendererHSV final : public allshader::WaveformRendererSignalBase {
  public:
    explicit WaveformRendererHSV(WaveformWidgetRenderer* waveformWidget);

    // override ::WaveformRendererSignalBase
    void onSetup(const QDomNode& node) override;

    void initializeGL() override;
    void paintGL() override;

  private:
    mixxx::RGBShader m_shader;
    VertexData m_vertices;
    RGBData m_colors;

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererHSV);
};
