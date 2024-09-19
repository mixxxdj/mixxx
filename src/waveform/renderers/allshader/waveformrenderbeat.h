#pragma once

#include <QColor>

#include "shaders/unicolorshader.h"
#include "util/class.h"
#include "waveform/renderers/allshader/vertexdata.h"
#include "waveform/renderers/allshader/waveformrenderer.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

class QDomNode;
class SkinContext;

namespace allshader {
class WaveformRenderBeat;
}

class allshader::WaveformRenderBeat final : public allshader::WaveformRenderer {
  public:
    explicit WaveformRenderBeat(WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play,
            WaveformRendererSignalBase::Options options = WaveformRendererSignalBase::Option::None);

    void setup(const QDomNode& node, const SkinContext& context) override;
    void paintGL() override;
    void initializeGL() override;

  private:
    mixxx::UnicolorShader m_beatShader;
    mixxx::UnicolorShader m_downbeatShader;
    mixxx::UnicolorShader m_markerbeatShader;
    QColor m_beatColor;
    QColor m_downbeatColor;
    QColor m_markerbeatColor;
    VertexData m_beatVertices;
    VertexData m_downbeatVertices;
    VertexData m_markerbeatVertices;

    bool m_isSlipRenderer;
    bool m_displayDownbeat;
    bool m_displayMarkerbeat;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBeat);
};
