#pragma once

#include <QColor>
#include <QImage>
#include <QSizeF>
#include <memory>

#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "rendergraph/geometrynode.h"
#include "rendergraph/node.h"
#include "track/bpmsegments.h"
#include "track/trackid.h"
#include "waveform/renderers/waveformrenderbpmcurvebase.h"

class QDomNode;

namespace rendergraph {
class GeometryNode;
class Context;
} // namespace rendergraph

namespace allshader {
class WaveformRenderBpmCurve;
} // namespace allshader

class WaveformBpmCurveNode;

struct SegmentPoint {
    double position;
    double duration;
    double bpm_start;
    double bpm_end;
    double range_start;
    double range_end;
    QString type;
};

struct BpmCurveStyle {
    QColor curveColor;
    int curveWidth;
    int curveOpacity;
    QColor markerColor;
    int markerWidth;
    int markerOpacity;
    Qt::PenStyle markerLineStyle;
    QColor labelTextColor;
    QColor labelBackgroundColor;
    int labelBackgroundOpacity;
    int labelFontSize;
    int labelDecimalPlaces;
    int labelOffset;
    QColor trackStartColor;
    int trackStartWidth;
    QColor offsetColor;
    int offsetWidth;
    Qt::PenStyle offsetLineStyle;
    bool showTrackStart;
    bool showOffsetIndicator;
    bool showDiamonds;

    BpmCurveStyle();
};

class allshader::WaveformRenderBpmCurve : public WaveformRenderBpmCurveBase,
                                          public rendergraph::Node {
    Q_OBJECT
  public:
    explicit WaveformRenderBpmCurve(WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play);
    ~WaveformRenderBpmCurve() override = default;

    void draw(QPainter* painter, QPaintEvent* event) override final;
    void setup(const QDomNode& node, const SkinContext& skinContext) override;
    bool init() override;
    void update() override;
    bool isSubtreeBlocked() const override;

    void setOffset(double offsetSeconds);
    double getOffset() const;

  protected:
    void updateBpmTextures() override;

  private:
    void createNode(const QImage& image);
    void updateNode();
    void loadBpmCurve();
    void calculateBpmRange();
    double mapBpmToY(double bpm, double height) const;
    double getFullImageWidth() const;
    QImage drawBpmTexture();
    QImage m_pendingBpmImage;
    TrackId m_lastTrackId;

    QVector<SegmentPoint> m_segments;
    BpmCurveStyle m_style;
    WaveformBpmCurveNode* m_pBpmCurveNode;
    rendergraph::Node* m_pBpmCurveNodesParent;
    std::unique_ptr<ControlProxy> m_pRateRatioCO;

    double m_minBpm;
    double m_maxBpm;
    double m_yMinBpm;
    double m_yMaxBpm;
    double m_offsetSeconds;
    double m_currentRateRatio;
    double m_trackLengthSeconds;
    double m_trackSamples;

    bool m_isSlipRenderer;
    bool m_textureReady;
    bool m_visible;
    bool m_needsTextureUpdate{true};
    bool m_needsRangeRecalculation{true};

    bool m_showBpmCurve = true;
    bool m_showBpmMarkers = true;
    bool m_showBpmLabels = true;

    QSizeF m_previousRendererSize;
};

inline BpmCurveStyle::BpmCurveStyle()
        : curveColor(100, 255, 100),
          curveWidth(2),
          curveOpacity(200),
          markerColor(255, 100, 100),
          markerWidth(1),
          markerOpacity(180),
          markerLineStyle(Qt::DashLine),
          labelTextColor(255, 100, 100),
          labelBackgroundColor(0, 0, 0),
          labelBackgroundOpacity(150),
          labelFontSize(20),
          labelDecimalPlaces(1),
          labelOffset(5),
          trackStartColor(100, 255, 100),
          trackStartWidth(1),
          offsetColor(255, 200, 100),
          offsetWidth(1),
          offsetLineStyle(Qt::DashDotLine),
          showTrackStart(true),
          showOffsetIndicator(true),
          showDiamonds(true) {
}
