#pragma once

#include <QColor>

#include "shaders/unicolorshader.h"
#include "util/class.h"
#include "waveform/renderers/allshader/vertexdata.h"
#include "waveform/renderers/allshader/waveformrenderer.h"

class QDomNode;
class SkinContext;

namespace allshader {
class WaveformRendererPreroll;
}

class allshader::WaveformRendererPreroll final : public allshader::WaveformRenderer {
  public:
    explicit WaveformRendererPreroll(WaveformWidgetRenderer* waveformWidgetRenderer);
    ~WaveformRendererPreroll() override;

    void setup(const QDomNode& node, const SkinContext& context) override;
    void renderGL() override;
    void initializeGL() override;

  private:
    mixxx::UnicolorShader m_shader;
    QColor m_color;
    VertexData m_vertices;

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererPreroll);
};
