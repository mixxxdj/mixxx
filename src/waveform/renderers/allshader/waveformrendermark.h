#pragma once

#include <QColor>

#include "rendergraph/geometrynode.h"
#include "rendergraph/node.h"
#include "waveform/renderers/waveformrendermarkbase.h"

class QDomNode;

namespace rendergraph {
class GeometryNode;
class Context;
} // namespace rendergraph

namespace allshader {
class DigitsRenderNode;
class WaveformRenderMark;
} // namespace allshader

class allshader::WaveformRenderMark : public ::WaveformRenderMarkBase,
                                      public rendergraph::Node {
  public:
    explicit WaveformRenderMark(WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play);

    // Pure virtual from WaveformRendererAbstract, not used
    void draw(QPainter* painter, QPaintEvent* event) override final;

    bool init() override;

    void update();

    bool isSubtreeBlocked() const override;

  private:
    void updateMarkImage(WaveformMarkPointer pMark) override;

    void updatePlayPosMarkTexture(rendergraph::Context* pContext);

    void drawTriangle(QPainter* painter,
            const QBrush& fillColor,
            QPointF p1,
            QPointF p2,
            QPointF p3);

    void updateUntilMark(double playPosition, double markerPosition);
    void updateDigitsNodeForUntilMark(float x);
    float getMaxHeightForText(float proportion) const;
    void updateRangeNode(rendergraph::GeometryNode* pNode,
            const QRectF& rect,
            QColor color);

    int m_beatsUntilMark;
    double m_timeUntilMark;
    double m_currentBeatPosition;
    double m_nextBeatPosition;
    std::unique_ptr<ControlProxy> m_pTimeRemainingControl;

    bool m_isSlipRenderer;

    rendergraph::Node* m_pRangeNodesParent{};
    rendergraph::Node* m_pMarkNodesParent{};

    rendergraph::GeometryNode* m_pPlayPosNode;
    float m_playPosHeight;
    float m_playPosDevicePixelRatio;

    DigitsRenderNode* m_pDigitsRenderNode{};

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMark);
};
