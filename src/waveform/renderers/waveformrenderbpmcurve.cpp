
#include "waveform/renderers/waveformrenderbpmcurve.h"

#include <QDebug>
#include <QDomNode>
#include <QPainter>
#include <QSizeF>
#include <algorithm>

#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "util/painterscope.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"

constexpr bool showDebugWaveformRenderBpmCurve = false;
static constexpr int kReloadCheckIntervalMs = 2000;

WaveformRenderBpmCurve::WaveformRenderBpmCurve(WaveformWidgetRenderer* renderer)
        : WaveformRendererAbstract(renderer),
          m_visible(true),
          m_minBpm(0),
          m_maxBpm(0),
          m_yMinBpm(0),
          m_yMaxBpm(0),
          m_offsetSeconds(0.0),
          m_currentRateRatio(1.0),
          m_pRateRatioCO(nullptr),
          m_reloadTimer(),
          m_previousRendererSize(0, 0) {
    initRateRatioControl();
    m_reloadTimer.start();

    PollingControlProxy proxyShowBpmCurve(QStringLiteral("[Waveform]"),
            QStringLiteral("show_bpm_curve"),
            ControlFlag::AllowMissingOrInvalid);
    m_showBpmCurve = proxyShowBpmCurve.get() > 0 ? true : false;

    PollingControlProxy proxyShowBpmMarkers(QStringLiteral("[Waveform]"),
            QStringLiteral("show_bpm_markers"),
            ControlFlag::AllowMissingOrInvalid);
    m_showBpmMarkers = proxyShowBpmMarkers.get() > 0 ? true : false;

    PollingControlProxy proxyShowBpmLabels(QStringLiteral("[Waveform]"),
            QStringLiteral("show_bpm_markers"),
            ControlFlag::AllowMissingOrInvalid);
    m_showBpmLabels = proxyShowBpmLabels.get() > 0 ? true : false;
}

void WaveformRenderBpmCurve::initRateRatioControl() {
    if (!m_waveformRenderer || m_waveformRenderer->getGroup().isEmpty()) {
        if (showDebugWaveformRenderBpmCurve) {
            qDebug() << "[WaveformRenderBpmCurve] Cannot init rate control - no group";
        }
        return;
    }

    if (showDebugWaveformRenderBpmCurve) {
        qDebug() << "[WaveformRenderBpmCurve] Creating rate control for group:"
                 << m_waveformRenderer->getGroup();
    }

    m_pRateRatioCO = std::make_unique<ControlProxy>(
            m_waveformRenderer->getGroup(), "rate_ratio");

    m_currentRateRatio = m_pRateRatioCO->get();
    if (showDebugWaveformRenderBpmCurve) {
        qDebug() << "[WaveformRenderBpmCurve] Initial rate ratio:" << m_currentRateRatio;
    }
}

void WaveformRenderBpmCurve::onRateRatioChanged(double value) {
    if (showDebugWaveformRenderBpmCurve) {
        qDebug() << "[WaveformRenderBpmCurve] Rate ratio changed to:" << value;
    }
    m_currentRateRatio = value;

    if (!m_segments.isEmpty()) {
        if (showDebugWaveformRenderBpmCurve) {
            qDebug() << "  First segment BPM original:" << m_segments[0].bpm_start;
            qDebug() << "  First segment BPM adjusted:"
                     << m_segments[0].bpm_start * m_currentRateRatio;
        }
    }

    // Recalculate Y-axis range with new rate ratio
    calculateBpmRange();
}

void WaveformRenderBpmCurve::setup(const QDomNode& node, const SkinContext& skinContext) {
    // Curve styling
    QString curveColorStr = skinContext.selectString(node, QStringLiteral("CurveColor"));
    if (!curveColorStr.isEmpty()) {
        m_style.curveColor = QColor(curveColorStr);
        m_style.curveColor = WSkinColor::getCorrectColor(m_style.curveColor).toRgb();
    }

    QString curveWidthStr = skinContext.selectString(node, QStringLiteral("CurveWidth"));
    if (!curveWidthStr.isEmpty()) {
        m_style.curveWidth = curveWidthStr.toInt();
    }

    QString curveOpacityStr = skinContext.selectString(node, QStringLiteral("CurveOpacity"));
    if (!curveOpacityStr.isEmpty()) {
        m_style.curveOpacity = curveOpacityStr.toInt();
    }
    m_style.curveColor.setAlpha(m_style.curveOpacity);

    // Marker styling
    QString markerColorStr = skinContext.selectString(node, QStringLiteral("MarkerColor"));
    if (!markerColorStr.isEmpty()) {
        m_style.markerColor = QColor(markerColorStr);
        m_style.markerColor = WSkinColor::getCorrectColor(m_style.markerColor).toRgb();
    }

    QString markerWidthStr = skinContext.selectString(node, QStringLiteral("MarkerWidth"));
    if (!markerWidthStr.isEmpty()) {
        m_style.markerWidth = markerWidthStr.toInt();
    }

    QString markerOpacityStr = skinContext.selectString(node, QStringLiteral("MarkerOpacity"));
    if (!markerOpacityStr.isEmpty()) {
        m_style.markerOpacity = markerOpacityStr.toInt();
    }
    m_style.markerColor.setAlpha(m_style.markerOpacity);

    QString markerLineStyleStr = skinContext.selectString(node, QStringLiteral("MarkerLineStyle"));
    if (!markerLineStyleStr.isEmpty()) {
        if (markerLineStyleStr == "solid") {
            m_style.markerLineStyle = Qt::SolidLine;
        } else if (markerLineStyleStr == "dash") {
            m_style.markerLineStyle = Qt::DashLine;
        } else if (markerLineStyleStr == "dot") {
            m_style.markerLineStyle = Qt::DotLine;
        } else if (markerLineStyleStr == "dashdot") {
            m_style.markerLineStyle = Qt::DashDotLine;
        }
    }

    // label styling
    QString labelTextColorStr = skinContext.selectString(node, QStringLiteral("LabelTextColor"));
    if (!labelTextColorStr.isEmpty()) {
        m_style.labelTextColor = QColor(labelTextColorStr);
        m_style.labelTextColor = WSkinColor::getCorrectColor(m_style.labelTextColor).toRgb();
    }

    QString labelBgColorStr = skinContext.selectString(
            node, QStringLiteral("LabelBackgroundColor"));
    if (!labelBgColorStr.isEmpty()) {
        m_style.labelBackgroundColor = QColor(labelBgColorStr);
        m_style.labelBackgroundColor =
                WSkinColor::getCorrectColor(m_style.labelBackgroundColor)
                        .toRgb();
    }

    QString labelBgOpacityStr = skinContext.selectString(
            node, QStringLiteral("LabelBackgroundOpacity"));
    if (!labelBgOpacityStr.isEmpty()) {
        m_style.labelBackgroundOpacity = labelBgOpacityStr.toInt();
    }
    m_style.labelBackgroundColor.setAlpha(m_style.labelBackgroundOpacity);

    QString labelFontSizeStr = skinContext.selectString(node, QStringLiteral("LabelFontSize"));
    if (!labelFontSizeStr.isEmpty()) {
        m_style.labelFontSize = labelFontSizeStr.toInt();
    }

    QString labelDecimalPlacesStr = skinContext.selectString(
            node, QStringLiteral("LabelDecimalPlaces"));
    if (!labelDecimalPlacesStr.isEmpty()) {
        m_style.labelDecimalPlaces = labelDecimalPlacesStr.toInt();
    }

    QString labelOffsetStr = skinContext.selectString(node, QStringLiteral("LabelOffset"));
    if (!labelOffsetStr.isEmpty()) {
        m_style.labelOffset = labelOffsetStr.toInt();
    }

    // track start styling
    QString trackStartColorStr = skinContext.selectString(node, QStringLiteral("TrackStartColor"));
    if (!trackStartColorStr.isEmpty()) {
        m_style.trackStartColor = QColor(trackStartColorStr);
        m_style.trackStartColor = WSkinColor::getCorrectColor(m_style.trackStartColor).toRgb();
    }

    QString trackStartWidthStr = skinContext.selectString(node, QStringLiteral("TrackStartWidth"));
    if (!trackStartWidthStr.isEmpty()) {
        m_style.trackStartWidth = trackStartWidthStr.toInt();
    }

    // offset styling
    QString offsetColorStr = skinContext.selectString(node, QStringLiteral("OffsetColor"));
    if (!offsetColorStr.isEmpty()) {
        m_style.offsetColor = QColor(offsetColorStr);
        m_style.offsetColor = WSkinColor::getCorrectColor(m_style.offsetColor).toRgb();
    }

    QString offsetWidthStr = skinContext.selectString(node, QStringLiteral("OffsetWidth"));
    if (!offsetWidthStr.isEmpty()) {
        m_style.offsetWidth = offsetWidthStr.toInt();
    }

    QString offsetLineStyleStr = skinContext.selectString(node, QStringLiteral("OffsetLineStyle"));
    if (!offsetLineStyleStr.isEmpty()) {
        if (offsetLineStyleStr == "solid") {
            m_style.offsetLineStyle = Qt::SolidLine;
        } else if (offsetLineStyleStr == "dash") {
            m_style.offsetLineStyle = Qt::DashLine;
        } else if (offsetLineStyleStr == "dot") {
            m_style.offsetLineStyle = Qt::DotLine;
        } else if (offsetLineStyleStr == "dashdot") {
            m_style.offsetLineStyle = Qt::DashDotLine;
        }
    }

    QString showTrackStartStr = skinContext.selectString(node, QStringLiteral("ShowTrackStart"));
    if (!showTrackStartStr.isEmpty()) {
        m_style.showTrackStart = (showTrackStartStr.compare("true", Qt::CaseInsensitive) == 0);
    }

    QString showOffsetIndicatorStr = skinContext.selectString(
            node, QStringLiteral("ShowOffsetIndicator"));
    if (!showOffsetIndicatorStr.isEmpty()) {
        m_style.showOffsetIndicator = (showOffsetIndicatorStr.compare("true",
                                               Qt::CaseInsensitive) == 0);
    }

    QString showDiamondsStr = skinContext.selectString(node, QStringLiteral("ShowDiamonds"));
    if (!showDiamondsStr.isEmpty()) {
        m_style.showDiamonds = (showDiamondsStr.compare("true", Qt::CaseInsensitive) == 0);
    }

    // offset from skin -< future
    QString offsetStr = skinContext.selectString(node, QStringLiteral("OffsetSeconds"));
    if (!offsetStr.isEmpty()) {
        m_offsetSeconds = offsetStr.toDouble();
    }

    loadBpmCurve();
    calculateBpmRange();
}

// Load BPM curve data from DB for the current track
void WaveformRenderBpmCurve::loadBpmCurve() {
    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();

    if (!pTrack) {
        if (showDebugWaveformRenderBpmCurve) {
            qDebug() << "[WaveformRenderBpmCurve] No track loaded";
        }
        return;
    }

    if (!pTrack->getId().isValid()) {
        if (showDebugWaveformRenderBpmCurve) {
            qDebug() << "[WaveformRenderBpmCurve] Track has no valid ID";
        }
        return;
    }

    if (showDebugWaveformRenderBpmCurve) {
        qDebug() << "[WaveformRenderBpmCurve] Loading BPM curve for track:"
                 << pTrack->getTitle()
                 << "ID:" << pTrack->getId().toString();
    }

    // Load from database via Track object
    QList<BpmSegmentsPointer> segments = pTrack->getBpmSegments();

    if (segments.isEmpty()) {
        if (showDebugWaveformRenderBpmCurve) {
            qDebug() << "[WaveformRenderBpmCurve] No BPM segments in database for track:"
                     << pTrack->getTitle()
                     << "ID:" << pTrack->getId().toString();
        }
        // Clear existing segments if any (in case they were reloaded)
        if (!m_segments.isEmpty()) {
            m_segments.clear();
        }
        return;
    }

    m_segments.clear();

    // Convert BpmSegments to SegmentPoint format
    for (const auto& pSegment : std::as_const(segments)) {
        SegmentPoint seg;

        seg.position = pSegment->getStartTime();
        seg.duration = pSegment->getDuration();
        seg.bpm_start = pSegment->getBpmStart();
        seg.bpm_end = pSegment->getBpmEnd();
        seg.range_start = pSegment->getRangeStart();
        seg.range_end = pSegment->getRangeEnd();
        seg.type = pSegment->getType();

        m_segments.append(seg);
    }

    if (showDebugWaveformRenderBpmCurve) {
        qDebug() << "[WaveformRenderBpmCurve] Loaded" << m_segments.size()
                 << "BPM segments from database for track:"
                 << pTrack->getTitle()
                 << "ID:" << pTrack->getId().toString();
    }

    // OFFSET STILL NEEDS TO BE ADDED IN DB
    m_offsetSeconds = 0.0;
    calculateBpmRange();
}

void WaveformRenderBpmCurve::onSetTrack() {
    if (showDebugWaveformRenderBpmCurve) {
        qDebug() << "[WaveformRenderBpmCurve] onSetTrack called";
    }

    // clear old bpmcurve
    m_segments.clear();
    m_minBpm = 0;
    m_maxBpm = 0;
    m_yMinBpm = 0;
    m_yMaxBpm = 0;
    m_reloadTimer.restart();
    loadBpmCurve();
    calculateBpmRange();
}

void WaveformRenderBpmCurve::calculateBpmRange() {
    if (m_segments.isEmpty()) {
        m_minBpm = 0;
        m_maxBpm = 0;
        m_yMinBpm = 0;
        m_yMaxBpm = 0;
        return;
    }

    double minBpm = m_segments[0].bpm_start * m_currentRateRatio;
    double maxBpm = m_segments[0].bpm_start * m_currentRateRatio;

    for (const auto& seg : std::as_const(m_segments)) {
        double startBpmAdj = seg.bpm_start * m_currentRateRatio;
        double endBpmAdj = seg.bpm_end * m_currentRateRatio;

        if (startBpmAdj < minBpm) {
            minBpm = startBpmAdj;
        }
        if (startBpmAdj > maxBpm) {
            maxBpm = startBpmAdj;
        }
        if (endBpmAdj < minBpm) {
            minBpm = endBpmAdj;
        }
        if (endBpmAdj > maxBpm) {
            maxBpm = endBpmAdj;
        }
    }

    m_minBpm = minBpm;
    m_maxBpm = maxBpm;

    // Calculate BPM range
    double bpmRange = m_maxBpm - m_minBpm;
    double minRange = 40.0; // Minimum 40 BPM on Y-axis

    double yMin, yMax;

    if (bpmRange < minRange) {
        // Center the range around the middle
        double midBpm = (m_minBpm + m_maxBpm) / 2.0;
        yMin = midBpm - (minRange / 2.0);
        yMax = midBpm + (minRange / 2.0);
    } else {
        // Add 10% padding for larger ranges
        double padding = bpmRange * 0.1;
        if (padding < 0.5) {
            padding = 0.5;
        }
        yMin = m_minBpm - padding;
        yMax = m_maxBpm + padding;
    }

    // no negative BPM
    if (yMin < 0) {
        yMin = 0;
    }

    m_yMinBpm = yMin;
    m_yMaxBpm = yMax;

    if (showDebugWaveformRenderBpmCurve) {
        qDebug() << "[WaveformRenderBpmCurve] BPM range (rate:" << m_currentRateRatio
                 << "):" << m_minBpm << "-" << m_maxBpm;
        qDebug() << "[WaveformRenderBpmCurve] Y-axis range:" << m_yMinBpm << "-" << m_yMaxBpm;
    }
}

void WaveformRenderBpmCurve::setOffset(double offsetSeconds) {
    m_offsetSeconds = offsetSeconds;
}

double WaveformRenderBpmCurve::getPositionWithOffset(double positionSeconds) const {
    return positionSeconds + m_offsetSeconds;
}

double WaveformRenderBpmCurve::mapBpmToY(
        double bpm, double yMinBpm, double yMaxBpm, double height) {
    // Apply rate ratio to BPM value
    double adjustedBpm = bpm * m_currentRateRatio;

    double bpmRange = yMaxBpm - yMinBpm;
    if (bpmRange <= 0.001) {
        return height / 2;
    }

    double normalized = (adjustedBpm - yMinBpm) / bpmRange;
    normalized = qBound(0.0, normalized, 1.0);
    return height - (normalized * height);
}

void WaveformRenderBpmCurve::drawLabel(QPainter* painter,
        const QPointF& position,
        double bpm,
        Qt::Orientation orientation) {
    // Apply rate ratio to displayed BPM value
    double adjustedBpm = bpm * m_currentRateRatio;
    QString labelText = QString::number(adjustedBpm, 'f', m_style.labelDecimalPlaces);

    QRect textRect = painter->fontMetrics().boundingRect(labelText);
    int padding = 4;

    QPointF labelPos;
    if (orientation == Qt::Horizontal) {
        labelPos = QPointF(position.x() - textRect.width() / 2 - padding,
                position.y() + m_style.labelOffset);

        if (labelPos.x() < 0) {
            labelPos.setX(0);
        } else if (labelPos.x() + textRect.width() + padding * 2 > painter->device()->width()) {
            labelPos.setX(painter->device()->width() - textRect.width() - padding * 2);
        }

        // Draw background if needed
        // if background rectangle is needed
        // QRect bgRect(static_cast<int>(labelPos.x() - padding),
        //        static_cast<int>(labelPos.y() - padding),
        //        textRect.width() + padding * 2,
        //        textRect.height() + padding * 2);
        // painter->setPen(Qt::NoPen);
        // painter->setBrush(m_style.labelBackgroundColor);
        // painter->drawRoundedRect(bgRect, 3, 3);

        // Draw textlabels
        painter->setPen(QPen(m_style.labelTextColor, 1));
        painter->drawText(static_cast<int>(labelPos.x()),
                static_cast<int>(labelPos.y() + textRect.height()),
                labelText);
    }
}

void WaveformRenderBpmCurve::draw(QPainter* painter, QPaintEvent* /*event*/) {
    // Check if renderer size has changed
    const float rendererWidth = m_waveformRenderer->getWidth();
    const float rendererHeight = m_waveformRenderer->getHeight();
    QSizeF currentRendererSize(rendererWidth, rendererHeight);

    bool sizeChanged = (currentRendererSize != m_previousRendererSize);
    if (sizeChanged) {
        m_previousRendererSize = currentRendererSize;
        if (showDebugWaveformRenderBpmCurve) {
            qDebug() << "[WaveformRenderBpmCurve] Renderer size changed to:"
                     << rendererWidth << "x" << rendererHeight;
        }
        QWidget* widget = dynamic_cast<QWidget*>(m_waveformRenderer);
        if (widget) {
            widget->update();
        }
        // Recalculate BPM range based on new size
        calculateBpmRange();
    }

    // if track is loaded -> Update play position and current bpm
    // if track had no bpmsegments on load -> wait while analyzing
    // after 2 secs try loading segments again
    // if track is not playing but maybe re-analyzed -> check every 2 secs

    if (m_reloadTimer.hasExpired(kReloadCheckIntervalMs)) {
        m_reloadTimer.restart();
        // Only reload if we don't have segments
        if (m_segments.isEmpty()) {
            if (showDebugWaveformRenderBpmCurve) {
                qDebug() << "[WaveformRenderBpmCurve] Periodic reload check - loading segments";
            }
            loadBpmCurve();
            // If we loaded segments now, we need to redraw
            if (!m_segments.isEmpty()) {
                update();
                return;
            }
        }
    }

    if (!m_visible || m_segments.isEmpty()) {
        return;
    }

    if (!m_visible || m_segments.isEmpty()) {
        return;
    }

    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        return;
    }

    // Update rate ratio from control proxy each frame atm
    if (m_pRateRatioCO) {
        double newRate = m_pRateRatioCO->get();
        if (std::abs(newRate - m_currentRateRatio) > 0.001) {
            if (showDebugWaveformRenderBpmCurve) {
                qDebug() << "[WaveformRenderBpmCurve] Rate ratio changed (polled):"
                         << m_currentRateRatio << "->" << newRate;
            }
            m_currentRateRatio = newRate;
            calculateBpmRange();
        }
    }

    double trackLengthSeconds = pTrack->getDuration();
    double trackSamples = m_waveformRenderer->getTrackSamples();

    if (trackLengthSeconds <= 0 || trackSamples <= 0) {
        return;
    }

    // Get visible range
    double firstDisplayedPosition = m_waveformRenderer->getFirstDisplayedPosition();
    double lastDisplayedPosition = m_waveformRenderer->getLastDisplayedPosition();

    firstDisplayedPosition = qMax(0.0, firstDisplayedPosition);
    lastDisplayedPosition = qMin(1.0, lastDisplayedPosition);

    if (firstDisplayedPosition >= lastDisplayedPosition) {
        firstDisplayedPosition = 0.0;
        lastDisplayedPosition = 1.0;
    }

    double startSample = firstDisplayedPosition * trackSamples;
    double endSample = lastDisplayedPosition * trackSamples;

    const Qt::Orientation orientation = m_waveformRenderer->getOrientation();

    PainterScope scope(painter);
    painter->setRenderHint(QPainter::Antialiasing);

    // Set up font for labels
    QFont labelFont = painter->font();
    labelFont.setPointSize(m_style.labelFontSize);
    labelFont.setBold(true);
    painter->setFont(labelFont);

    // draw curve STABLE / INCREASE / DECREASE
    if (m_showBpmCurve) {
        painter->setPen(QPen(m_style.curveColor, m_style.curveWidth));

        QVector<QLineF> curveLines;

        for (const auto& seg : std::as_const(m_segments)) {
            // Skip invalid segments
            if (seg.duration <= 0.001) {
                continue;
            }

            double startTime = seg.position + m_offsetSeconds;
            double endTime = seg.range_end + m_offsetSeconds;

            // Clamp times to track bounds
            startTime = qMax(0.0, qMin(startTime, trackLengthSeconds));
            endTime = qMax(0.0, qMin(endTime, trackLengthSeconds));

            if (endTime <= startTime) {
                continue;
            }

            double startBpm = seg.bpm_start;
            double endBpm = seg.bpm_end;

            // sample positions
            double startPos = startTime * (trackSamples / trackLengthSeconds);
            double endPos = endTime * (trackSamples / trackLengthSeconds);

            // Clamp to valid sample range
            startPos = qMax(0.0, qMin(startPos, trackSamples));
            endPos = qMax(0.0, qMin(endPos, trackSamples));

            // outside visible range
            if (endPos < startSample || startPos > endSample) {
                continue;
            }

            double x1 = m_waveformRenderer->transformSamplePositionInRendererWorld(
                    startPos, ::WaveformRendererAbstract::Play);
            double x2 = m_waveformRenderer->transformSamplePositionInRendererWorld(
                    endPos, ::WaveformRendererAbstract::Play);
            double y1 = mapBpmToY(startBpm, m_yMinBpm, m_yMaxBpm, rendererHeight);
            double y2 = mapBpmToY(endBpm, m_yMinBpm, m_yMaxBpm, rendererHeight);

            if (orientation == Qt::Vertical) {
                std::swap(x1, y1);
                std::swap(x2, y2);
            }

            x1 = qBound(0.0, x1, (double)rendererWidth);
            x2 = qBound(0.0, x2, (double)rendererWidth);
            y1 = qBound(0.0, y1, (double)rendererHeight);
            y2 = qBound(0.0, y2, (double)rendererHeight);

            curveLines.append(QLineF(x1, y1, x2, y2));
        }

        if (!curveLines.isEmpty()) {
            painter->drawLines(curveLines);
        }
    }
    // draw markers
    if (m_showBpmMarkers) {
        painter->setPen(QPen(m_style.markerColor, m_style.markerWidth, m_style.markerLineStyle));

        QVector<QLineF> markerLines;
        QVector<std::pair<QPointF, double>> labelPositions;

        for (const auto& seg : std::as_const(m_segments)) {
            double boundaryTime = seg.position + m_offsetSeconds;
            double boundaryPos = boundaryTime * (trackSamples / trackLengthSeconds);

            if (boundaryPos >= startSample && boundaryPos <= endSample) {
                double x = m_waveformRenderer->transformSamplePositionInRendererWorld(
                        boundaryPos, ::WaveformRendererAbstract::Play);

                if (x >= 0 && x <= rendererWidth) {
                    if (orientation == Qt::Horizontal) {
                        markerLines.append(QLineF(x, 0, x, rendererHeight));
                        labelPositions.append({QPointF(x, 0), seg.bpm_start});
                    } else {
                        markerLines.append(QLineF(0, x, rendererWidth, x));
                        labelPositions.append({QPointF(0, x), seg.bpm_start});
                    }
                }
            }
        }

        if (!markerLines.isEmpty()) {
            painter->drawLines(markerLines);
        }

        // draw marker symbols
        if (m_style.showDiamonds && !markerLines.isEmpty()) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(m_style.markerColor);
            for (const QLineF& line : std::as_const(markerLines)) {
                QPointF topPoint = line.p1();
                QPolygonF diamond;
                diamond << QPointF(topPoint.x(), topPoint.y() - 4)
                        << QPointF(topPoint.x() + 4, topPoint.y())
                        << QPointF(topPoint.x(), topPoint.y() + 4)
                        << QPointF(topPoint.x() - 4, topPoint.y());
                painter->drawPolygon(diamond);
            }
        }

        // draw labels
        if (m_showBpmLabels) {
            for (const auto& labelInfo : std::as_const(labelPositions)) {
                drawLabel(painter, labelInfo.first, labelInfo.second, orientation);
            }
        }
    }

    // draw track start
    if (m_style.showTrackStart) {
        painter->setPen(QPen(m_style.trackStartColor, m_style.trackStartWidth, Qt::SolidLine));
        double trackStartX = m_waveformRenderer->transformSamplePositionInRendererWorld(
                0.0, ::WaveformRendererAbstract::Play);
        if (trackStartX >= 0 && trackStartX <= rendererWidth) {
            if (orientation == Qt::Horizontal) {
                painter->drawLine(static_cast<int>(trackStartX),
                        0,
                        static_cast<int>(trackStartX),
                        static_cast<int>(rendererHeight));
            } else {
                painter->drawLine(0,
                        static_cast<int>(trackStartX),
                        static_cast<int>(rendererWidth),
                        static_cast<int>(trackStartX));
            }
        }
    }

    // draw segment end marker with BPM label (at the end of the last segment)
    if (m_style.showTrackStart && !m_segments.isEmpty()) {
        painter->setPen(QPen(m_style.trackStartColor, m_style.trackStartWidth, Qt::SolidLine));

        // Get the last segment
        const auto& lastSegment = m_segments.last();
        double endBpm = lastSegment.bpm_end;
        double endTime = lastSegment.range_end + m_offsetSeconds;

        // Convert to sample position
        double endPos = endTime * (trackSamples / trackLengthSeconds);
        double endX = m_waveformRenderer->transformSamplePositionInRendererWorld(
                endPos, ::WaveformRendererAbstract::Play);

        if (endX >= 0 && endX <= rendererWidth) {
            if (orientation == Qt::Horizontal) {
                painter->drawLine(static_cast<int>(endX),
                        0,
                        static_cast<int>(endX),
                        static_cast<int>(rendererHeight));
            } else {
                painter->drawLine(0,
                        static_cast<int>(endX),
                        static_cast<int>(rendererWidth),
                        static_cast<int>(endX));
            }

            // Draw BPM label at the end marker
            if (m_showBpmLabels) {
                QPointF labelPos;
                if (orientation == Qt::Horizontal) {
                    labelPos = QPointF(endX - 30, m_style.labelOffset);
                    drawLabel(painter, labelPos, endBpm, orientation);
                } else {
                    labelPos = QPointF(m_style.labelOffset, endX - 30);
                    drawLabel(painter, labelPos, endBpm, orientation);
                }
            }
        }
    }

    // draw offset
    if (m_style.showOffsetIndicator && !m_segments.isEmpty() && std::abs(m_offsetSeconds) > 0.001) {
        painter->setPen(QPen(m_style.offsetColor, m_style.offsetWidth, m_style.offsetLineStyle));

        double firstOffsetPosition = m_segments[0].position + m_offsetSeconds;
        double offsetSample = firstOffsetPosition * (trackSamples / trackLengthSeconds);
        double offsetX = m_waveformRenderer->transformSamplePositionInRendererWorld(
                offsetSample, ::WaveformRendererAbstract::Play);

        if (offsetX >= 0 && offsetX <= rendererWidth) {
            if (orientation == Qt::Horizontal) {
                painter->drawLine(static_cast<int>(offsetX),
                        0,
                        static_cast<int>(offsetX),
                        static_cast<int>(rendererHeight));
            } else {
                painter->drawLine(0,
                        static_cast<int>(offsetX),
                        static_cast<int>(rendererWidth),
                        static_cast<int>(offsetX));
            }
        }
    }
}
