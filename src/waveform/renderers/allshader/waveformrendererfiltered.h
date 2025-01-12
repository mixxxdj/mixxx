#pragma once

#include "shaders/unicolorshader.h"
#include "util/class.h"
#include "waveform/renderers/allshader/vertexdata.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

namespace allshader {
class WaveformRendererFiltered;
}

class allshader::WaveformRendererFiltered final : public allshader::WaveformRendererSignalBase {
  public:
    explicit WaveformRendererFiltered(WaveformWidgetRenderer* waveformWidget, bool rgbStacked);

    // override ::WaveformRendererSignalBase
    void onSetup(const QDomNode& node) override;

    void initializeGL() override;
    void paintGL() override;

  private:
    const bool m_bRgbStacked;
    mixxx::UnicolorShader m_shader;
    VertexData m_vertices[4];

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererFiltered);
};
