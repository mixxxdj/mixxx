#include "waveform/renderers/waveformbeat.h"

#include "util/math.h"

namespace {
constexpr int kTriangleEdgeLength = 10;
constexpr int kClickableLinePaddingPixels = 5;

inline float getEquilateralTriangleAltitude(float triangleEdgeLength) {
    return sqrt(3) / 2 * triangleEdgeLength;
}

QPolygonF getEquilateralTriangle(float edgeLength,
        QPointF baseMidPoint,
        Direction pointingDirection) {
    QPolygonF polygon;
    QPointF firstPoint = baseMidPoint;
    QPointF secondPoint = baseMidPoint;
    QPointF thirdPoint = baseMidPoint;

    float halfBase = edgeLength / 2;
    float altitude = getEquilateralTriangleAltitude(edgeLength);

    if (pointingDirection == Direction::DOWN || pointingDirection == Direction::UP) {
        firstPoint.setX(baseMidPoint.x() - halfBase);
        secondPoint.setX(baseMidPoint.x() + halfBase);
        if (pointingDirection == Direction::DOWN) {
            thirdPoint.setY(baseMidPoint.y() + altitude);
        } else {
            thirdPoint.setY(baseMidPoint.y() - altitude);
        }
    } else {
        firstPoint.setY(baseMidPoint.y() - halfBase);
        secondPoint.setY(baseMidPoint.y() + halfBase);
        if (pointingDirection == Direction::RIGHT) {
            thirdPoint.setX(baseMidPoint.x() + altitude);
        } else {
            thirdPoint.setX(baseMidPoint.x() - altitude);
        }
    }

    polygon << firstPoint << secondPoint << thirdPoint;
    return polygon;
}
} // namespace

WaveformBeat::WaveformBeat()
        : m_orientation(Qt::Horizontal),
          m_beatGridMode(BeatGridMode::BEATS_DOWNBEATS),
          m_beat(mixxx::FramePos(0)),
          m_bVisible(true) {
}

void WaveformBeat::draw(QPainter* painter) const {
    if (m_bVisible) {
        if (m_orientation == Qt::Horizontal) {
            painter->drawLine(QPointF(m_position, 0), QPoint(m_position, m_length));
            if (m_beat.getType() == mixxx::Beat::DOWNBEAT &&
                    m_beatGridMode == BeatGridMode::BEATS_DOWNBEATS) {
                // TODO(hacksdump): Get color from skin context
                painter->setBrush(Qt::red);
                painter->drawPolygon(getEquilateralTriangle(
                        kTriangleEdgeLength, QPointF(m_position, 0), Direction::DOWN));
                painter->drawPolygon(getEquilateralTriangle(
                        kTriangleEdgeLength, QPointF(m_position, m_length), Direction::UP));
            }
        } else {
            painter->drawLine(QPointF(0, m_position), QPoint(m_length, m_position));
            if (m_beat.getType() == mixxx::Beat::BEAT &&
                    m_beatGridMode == BeatGridMode::BEATS_DOWNBEATS) {
                painter->setBrush(Qt::red);
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

bool operator<(const WaveformBeat& beat1, const WaveformBeat& beat2) {
    return beat1.getBeat() < beat2.getBeat();
}
