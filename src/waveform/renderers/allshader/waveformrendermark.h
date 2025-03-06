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
    Q_OBJECT
  public:
    explicit WaveformRenderMark(WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play);

    // Pure virtual from WaveformRendererAbstract, not used
    void draw(QPainter* painter, QPaintEvent* event) override final;

    void setup(const QDomNode& node, const SkinContext& skinContext) override;

    bool init() override;

    void update() override;

    bool isSubtreeBlocked() const override;

  public slots:
    void setPlayMarkerForegroundColor(const QColor& fgPlayColor) {
        m_playMarkerForegroundColor = fgPlayColor;
    }
    void setPlayMarkerBackgroundColor(const QColor& bgPlayColor) {
        m_playMarkerBackgroundColor = bgPlayColor;
    }
    void setUntilMarkShowBeats(bool untilMarkShowBeats) {
        m_untilMarkShowBeats = untilMarkShowBeats;
    }
    void setUntilMarkShowTime(bool untilMarkShowTime) {
        m_untilMarkShowTime = untilMarkShowTime;
    }
    void setUntilMarkAlign(Qt::Alignment untilMarkAlign) {
        m_untilMarkAlign = untilMarkAlign;
    }
    void setUntilMarkTextSize(int untilMarkTextSize) {
        m_untilMarkTextSize = untilMarkTextSize;
    }
    void setUntilMarkTextHeightLimit(float untilMarkTextHeightLimit) {
        m_untilMarkTextHeightLimit = untilMarkTextHeightLimit;
    }

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

    QColor m_playMarkerForegroundColor;
    QColor m_playMarkerBackgroundColor;

    bool m_untilMarkShowBeats;
    bool m_untilMarkShowTime;
    Qt::Alignment m_untilMarkAlign;
    int m_untilMarkTextSize;
    float m_untilMarkTextHeightLimit;

    DISALLOW_COPY_AND_ASSIGN(WaveformRenderMark);
};
