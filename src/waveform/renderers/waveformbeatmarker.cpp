#include "waveform/renderers/waveformbeatmarker.h"

#include "waveform/renderers/geometryutils.h"

namespace {
constexpr int kTriangleEdgeLength = 15;
} // namespace

WaveformBeatMarker::WaveformBeatMarker()
        : m_orientation(Qt::Horizontal) {
}

void WaveformBeatMarker::draw(QPainter* painter) const {
    painter->setBrush(Qt::white);
    painter->setPen((QPen(Qt::white, 2)));
    if (m_orientation == Qt::Horizontal) {
        painter->drawLine(QPointF(m_position, 0), QPoint(m_position, m_length));
        painter->drawPolygon(getEquilateralTriangle(
                kTriangleEdgeLength, QPointF(m_position, 0), Direction::DOWN));
    } else {
        painter->drawLine(QPointF(0, m_position), QPoint(m_length, m_position));
        painter->setBrush(Qt::white);
        painter->drawPolygon(getEquilateralTriangle(
                kTriangleEdgeLength, QPointF(0, m_position), Direction::RIGHT));
    }
}
