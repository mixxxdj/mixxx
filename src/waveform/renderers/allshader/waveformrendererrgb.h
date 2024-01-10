#pragma once

#include "shaders/rgbashader.h"
#include "shaders/rgbshader.h"
#include "util/class.h"
#include "waveform/renderers/allshader/rgbadata.h"
#include "waveform/renderers/allshader/rgbdata.h"
#include "waveform/renderers/allshader/vertexdata.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

namespace allshader {
class WaveformRendererRGB;
}

class allshader::WaveformRendererRGB final : public allshader::WaveformRendererSignalBase {
  public:
    explicit WaveformRendererRGB(WaveformWidgetRenderer* waveformWidget);

    // override ::WaveformRendererSignalBase
    void onSetup(const QDomNode& node) override;

    void initializeGL() override;
    void paintGL() override;

  private:
    mixxx::RGBShader m_shader;
    mixxx::RGBAShader m_shader_ghost;
    VertexData m_vertices;
    RGBData m_colors;
    VertexData m_vertices_ghost;
    RGBAData m_colors_ghost;

    const float ghost_alpha = 0.25f;

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererRGB);
};
