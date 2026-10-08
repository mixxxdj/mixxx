#pragma once

#include <QColor>
#include <QDateTime>
#include <QElapsedTimer>
#include <QFont>
#include <QLineF>
#include <QSizeF>
#include <QVector>

#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
#include "track/bpmsegments.h"
#include "waveform/renderers/waveformrendererabstract.h"

struct SegmentPoint {
    double position;
    double duration;
    double bpm_start;
    double bpm_end;
    double range_start;
    double range_end;
    QString type; // "STABLE", "INCREASE", "DECREASE"
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

    BpmCurveStyle() {
        curveColor = QColor(100, 255, 100);
        curveWidth = 2;
        curveOpacity = 200;

        markerColor = QColor(255, 100, 100);
        markerWidth = 1;
        markerOpacity = 180;
        markerLineStyle = Qt::DashLine;

        labelTextColor = QColor(255, 100, 100);
        labelBackgroundColor = QColor(0, 0, 0);
        labelBackgroundOpacity = 150;
        // labelFontSize = 8;
        labelFontSize = 20;
        labelDecimalPlaces = 1;
        labelOffset = 5;

        trackStartColor = QColor(100, 255, 100);
        trackStartWidth = 1;

        offsetColor = QColor(255, 200, 100);
        offsetWidth = 1;
        offsetLineStyle = Qt::DashDotLine;

        showTrackStart = true;
        showOffsetIndicator = true;
        showDiamonds = true;
    }
};

class WaveformRenderBpmCurve : public WaveformRendererAbstract {
  public:
    explicit WaveformRenderBpmCurve(WaveformWidgetRenderer* renderer);
    ~WaveformRenderBpmCurve() override = default;

    void setup(const QDomNode& node, const SkinContext& context) override;
    void draw(QPainter* painter, QPaintEvent* event) override;
    void onSetTrack() override;

    // style
    void setStyle(const BpmCurveStyle& style) {
        m_style = style;
    }
    BpmCurveStyle getStyle() const {
        return m_style;
    }

    // offset
    void setOffset(double offsetSeconds);
    double getOffset() const {
        return m_offsetSeconds;
    }

    // visibility
    void setVisible(bool visible) {
        m_visible = visible;
    }
    bool isVisible() const {
        return m_visible;
    }

  private:
    QVector<SegmentPoint> m_segments;
    BpmCurveStyle m_style;
    bool m_visible;
    double m_minBpm;
    double m_maxBpm;
    double m_yMinBpm;
    double m_yMaxBpm;
    double m_offsetSeconds;
    double m_currentRateRatio;
    std::unique_ptr<ControlProxy> m_pRateRatioCO;

    void initRateRatioControl();
    void onRateRatioChanged(double value);
    void loadBpmCurve();
    double getPositionWithOffset(double positionSeconds) const;
    double mapBpmToY(double bpm, double yMinBpm, double yMaxBpm, double height);
    void calculateBpmRange();
    void drawLabel(QPainter* painter,
            const QPointF& position,
            double bpm,
            Qt::Orientation orientation);
    QDateTime m_lastLoadTime;
    QElapsedTimer m_reloadTimer;

    bool m_showBpmCurve = true;
    bool m_showBpmMarkers = true;
    bool m_showBpmLabels = true;

    QSizeF m_previousRendererSize;
};
