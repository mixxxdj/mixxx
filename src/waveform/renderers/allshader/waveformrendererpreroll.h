#pragma once

#include <QColor>

#include "rendergraph/geometrynode.h"
#include "util/class.h"
#include "waveform/renderers/waveformrendererabstract.h"

class QDomNode;
class SkinContext;

namespace allshader {
class WaveformRendererPreroll;
} // namespace allshader

class allshader::WaveformRendererPreroll final
        : public ::WaveformRendererAbstract,
          public rendergraph::GeometryNode {
  public:
    explicit WaveformRendererPreroll(
            WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play);
    ~WaveformRendererPreroll() override;

    // Pure virtual from WaveformRendererAbstract, not used
    void draw(QPainter* painter, QPaintEvent* event) override final;

    void setup(const QDomNode& node, const SkinContext& skinContext) override;

    // Virtual for rendergraph::Node
    void preprocess() override;

  private:
    QColor m_color;
    float m_markerBreadth{};
    float m_markerLength{};
    bool m_isSlipRenderer;

    bool preprocessInner();

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererPreroll);
};
