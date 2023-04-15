#pragma once

#include "util/class.h"
#include "waveform/renderers/qopengl/shaders/unicolorshader.h"
#include "waveform/renderers/qopengl/vertexdata.h"
#include "waveform/renderers/qopengl/waveformrenderersignalbase.h"

namespace qopengl {
class WaveformRendererFiltered;
}

class qopengl::WaveformRendererFiltered final : public qopengl::WaveformRendererSignalBase {
  public:
    explicit WaveformRendererFiltered(WaveformWidgetRenderer* waveformWidget);
    ~WaveformRendererFiltered() override;

    // override ::WaveformRendererSignalBase
    void onSetup(const QDomNode& node) override;

    void initializeGL() override;
    void renderGL() override;

  private:
    UnicolorShader m_shader;
    VertexData m_verticesForGroup[4];

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererFiltered);
};
