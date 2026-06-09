#pragma once

#include "rendergraph/node.h"
#include "util/class.h"
#include "waveform/renderers/waveformrendererabstract.h"

namespace allshader {
class WaveformRenderPhrase;
} // namespace allshader

class allshader::WaveformRenderPhrase final
        : public ::WaveformRendererAbstract,
          public rendergraph::Node {
  public:
    explicit WaveformRenderPhrase(WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play);

    void draw(QPainter* painter, QPaintEvent* event) override final;

    void setup(const QDomNode& node, const SkinContext& skinContext) override;

    void preprocess() override;

  private:
    bool m_isSlipRenderer;

    bool preprocessInner();

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderPhrase);
};
