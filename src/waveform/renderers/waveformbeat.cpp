#include "waveform/renderers/waveformbeat.h"

#include <math.h>

WaveformBeat::WaveformBeat()
        : m_orientation(Qt::Horizontal), m_beatType(mixxx::track::io::BEAT), m_beatGridMode(1) {
}

void WaveformBeat::draw(QPainter* painter) const {
    const float edgeLength = 10;

    if (m_orientation == Qt::Horizontal) {
        painter->drawLine(QPointF(m_position, 0), QPoint(m_position, m_length));
        if (m_beatType == mixxx::track::io::BAR && m_beatGridMode == 1) {
            // TODO: Get color from skin context
            painter->setBrush(Qt::red);
            painter->drawPolygon(getEquilateralTriangle(
                    edgeLength, QPointF(m_position, 0), Direction::DOWN));
            painter->drawPolygon(getEquilateralTriangle(
                    edgeLength, QPointF(m_position, m_length), Direction::UP));
        }
    } else {
        painter->drawLine(QPointF(0, m_position), QPoint(m_length, m_position));
        if (m_beatType == mixxx::track::io::BAR && m_beatGridMode == 1) {
            painter->setBrush(Qt::red);
            painter->drawPolygon(getEquilateralTriangle(
                    edgeLength, QPointF(0, m_position), Direction::RIGHT));
            painter->drawPolygon(getEquilateralTriangle(edgeLength,
                    QPointF(m_length, m_position),
                    Direction::LEFT));
        }
    }
}

QPolygonF WaveformBeat::getEquilateralTriangle(float edgeLength,
        QPointF baseMidPoint,
        Direction pointingDirection) const {
    QPolygonF polygon;
    QPointF firstPoint = baseMidPoint;
    QPointF secondPoint = baseMidPoint;
    QPointF thirdPoint = baseMidPoint;

    float halfBase = edgeLength / 2;
    float altitude = sqrt(3) / 2 * edgeLength;

    if (m_orientation == Qt::Horizontal) {
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
        if (pointingDirection == Direction::DOWN) {
            thirdPoint.setX(baseMidPoint.x() + altitude);
        } else {
            thirdPoint.setY(baseMidPoint.x() - altitude);
        }
    }

    polygon << firstPoint << secondPoint << thirdPoint;
    return polygon;
}
