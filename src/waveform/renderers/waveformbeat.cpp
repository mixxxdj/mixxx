#include "waveform/renderers/waveformbeat.h"

#include "waveform/renderers/geometryutils.h"

namespace {
constexpr int kTriangleEdgeLength = 6;
constexpr int kClickableLinePaddingPixels = 5;
const QColor downbeatTriangleFillBaseColor = Qt::red;
constexpr int fontSizePixels = 10;
} // namespace

WaveformBeat::WaveformBeat()
        : m_beat(mixxx::kStartFramePos),
          m_orientation(Qt::Horizontal),
          m_iAlpha(255),
          m_beatGridMode(kDefaultBeatGridMode),
          m_bVisible(true) {
}

void WaveformBeat::draw(QPainter* painter) const {
    if (m_bVisible) {
        auto downBeatTriangleColor = downbeatTriangleFillBaseColor;
        downBeatTriangleColor.setAlpha(m_iAlpha);
        painter->setBrush(downBeatTriangleColor);
        QFont font = painter->font();
        font.setPixelSize(fontSizePixels);
        painter->setFont(font);
        const int downbeatNumberGapPixels = 5;
        if (m_orientation == Qt::Horizontal) {
            painter->drawLine(QPointF(m_position, 0), QPoint(m_position, m_length));
            if (m_beat.type() == mixxx::BeatType::Downbeat &&
                    m_beatGridMode == BeatGridMode::BeatsAndDownbeats) {
                painter->setPen(Qt::white);
                painter->drawText(m_position + downbeatNumberGapPixels,
                        painter->fontMetrics().height(),
                        QString::number(m_beat.barIndex() + 1));
                painter->setPen(Qt::transparent);
                painter->drawPolygon(getEquilateralTriangle(
                        kTriangleEdgeLength, QPointF(m_position, 0), Direction::DOWN));
                painter->drawPolygon(getEquilateralTriangle(
                        kTriangleEdgeLength, QPointF(m_position, m_length), Direction::UP));
            }
        } else {
            painter->drawLine(QPointF(0, m_position), QPoint(m_length, m_position));
            if (m_beat.type() == mixxx::BeatType::Downbeat &&
                    m_beatGridMode == BeatGridMode::BeatsAndDownbeats) {
                painter->setPen(Qt::white);
                painter->drawText(downbeatNumberGapPixels,
                        m_position - downbeatNumberGapPixels,
                        QString::number(m_beat.barIndex() + 1));
                painter->setPen(Qt::transparent);
                painter->drawPolygon(getEquilateralTriangle(
                        kTriangleEdgeLength, QPointF(0, m_position), Direction::RIGHT));
                painter->drawPolygon(getEquilateralTriangle(kTriangleEdgeLength,
                        QPointF(m_length, m_position),
                        Direction::LEFT));
            }
        }
    }
}

bool WaveformBeat::contains(QPoint point, Qt::Orientation orientation) const {
    Q_UNUSED(orientation);
    int paddedPositionPixelsBeforeLine = m_position - kClickableLinePaddingPixels;
    int paddedPositionPixelsAfterLine = m_position + kClickableLinePaddingPixels;
    if (m_orientation == Qt::Horizontal) {
        return paddedPositionPixelsBeforeLine < point.x() &&
                point.x() < paddedPositionPixelsAfterLine;
    } else {
        return paddedPositionPixelsBeforeLine < point.y() &&
                point.y() < paddedPositionPixelsAfterLine;
    }
}
