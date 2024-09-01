#pragma once

#include <QColor>

#include "rendergraph/node.h"
#include "waveform/renderers/waveformrendermarkbase.h"

class QDomNode;
class SkinContext;

namespace rendergraph {
class GeometryNode;
}

namespace allshader {
class DigitsRenderNode;
class WaveformRenderMark;
}

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

    // Virtual for rendergraph::Node
    void initialize() override;
    void resize(int, int) override;
    bool isSubtreeBlocked() const override;

  private:
    void updateMarkImage(WaveformMarkPointer pMark) override;

    void updatePlayPosMarkTexture();

    void drawTriangle(QPainter* painter,
            const QBrush& fillColor,
            QPointF p1,
            QPointF p2,
            QPointF p3);

    void drawMark(const QMatrix4x4& matrix, const QRectF& rect, QColor color);
    void updateUntilMark(double playPosition, double markerPosition);
    void drawUntilMark(const QMatrix4x4& matrix, float x);
    float getMaxHeightForText() const;
    void updateRangeNode(rendergraph::GeometryNode* pNode,
            const QMatrix4x4& matrix,
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
    DigitsRenderNode* m_pDigitsRenderNode{};

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMark);
};
