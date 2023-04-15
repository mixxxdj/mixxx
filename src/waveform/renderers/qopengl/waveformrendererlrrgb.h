#pragma once

#include "util/class.h"
#include "waveform/renderers/qopengl/colordata.h"
#include "waveform/renderers/qopengl/shaders/colorshader.h"
#include "waveform/renderers/qopengl/vertexdata.h"
#include "waveform/renderers/qopengl/waveformrenderersignalbase.h"

namespace qopengl {
class WaveformRendererLRRGB;
}

class qopengl::WaveformRendererLRRGB final : public qopengl::WaveformRendererSignalBase {
  public:
    explicit WaveformRendererLRRGB(WaveformWidgetRenderer* waveformWidget);
    ~WaveformRendererLRRGB() override;

    // override ::WaveformRendererSignalBase
    void onSetup(const QDomNode& node) override;

    void initializeGL() override;
    void renderGL() override;

  private:
    ColorShader m_shader;
    VertexData m_vertices;
    ColorData m_colors;

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererLRRGB);
};
