#pragma once

#include <QColor>

#include "rendergraph/node.h"
#include "util/class.h"
#include "waveform/renderers/waveformrendererabstract.h"

class QDomNode;
class SkinContext;

namespace rendergraph {
class GeometryNode;
class Context;
} // namespace rendergraph

namespace allshader {
class WaveformRenderBarCounter;
} // namespace allshader

class allshader::WaveformRenderBarCounter final
        : public QObject,
          public ::WaveformRendererAbstract,
          public rendergraph::Node {
    Q_OBJECT
  public:
    explicit WaveformRenderBarCounter(WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play);

    // Pure virtual from WaveformRendererAbstract, not used
    void draw(QPainter* painter, QPaintEvent* event) override final;

    void setup(const QDomNode& node, const SkinContext& skinContext) override;

    // Virtual for rendergraph::Node
    void preprocess() override;

  public slots:
    void setColor(const QColor& color) {
        m_color = color;
    }
    void setBeatsPerBar(int beatsPerBar) {
        m_beatsPerBar = beatsPerBar;
    }

  private:
    bool preprocessInner();
    void removeAllChildNodes();

    QColor m_color;
    int m_beatsPerBar{4};
    bool m_isSlipRenderer;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderBarCounter);
};
