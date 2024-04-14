#pragma once

#include <QColor>

#include "shaders/unicolorshader.h"
#include "util/class.h"
#include "waveform/renderers/allshader/vertexbuffer.h"
#include "waveform/renderers/allshader/waveformrenderer.h"

class QDomNode;
class SkinContext;

namespace allshader {
class WaveformRenderBeat;
}

class allshader::WaveformRenderBeat final : public allshader::WaveformRenderer {
  public:
    explicit WaveformRenderBeat(WaveformWidgetRenderer* waveformWidget);

    void setup(const QDomNode& node, const SkinContext& context) override;
    void paintGL() override;
    void initializeGL() override;

  private:
    mixxx::UnicolorShader m_shader;
    QColor m_color;
    Vector2DVertexBuffer m_vertices;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBeat);
};
