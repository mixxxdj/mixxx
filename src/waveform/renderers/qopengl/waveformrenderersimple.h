#pragma once

#include "util/class.h"
#include "waveform/renderers/qopengl/shaders/unicolorshader.h"
#include "waveform/renderers/qopengl/vertexdata.h"
#include "waveform/renderers/qopengl/waveformrenderersignalbase.h"

namespace qopengl {
class WaveformRendererSimple;
}

class qopengl::WaveformRendererSimple final : public qopengl::WaveformRendererSignalBase {
  public:
    explicit WaveformRendererSimple(WaveformWidgetRenderer* waveformWidget);
    ~WaveformRendererSimple() override;

    // override ::WaveformRendererSignalBase
    void onSetup(const QDomNode& node) override;

    void initializeGL() override;
    void renderGL() override;

  private:
    UnicolorShader m_shader;
    VertexData m_vertices[2];

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererSimple);
};
