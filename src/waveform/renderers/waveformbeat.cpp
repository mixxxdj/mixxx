#include <math.h>
#include <waveform/renderers/waveformbeat.h>

WaveformBeat::WaveformBeat()
        : m_orientation(Qt::Horizontal), m_beatType(mixxx::track::io::BEAT) {
}

void WaveformBeat::draw(QPainter* painter) const {
    painter->drawLine(QPointF(m_position, 0), QPoint(m_position, m_length));
    if (m_beatType == mixxx::track::io::BAR) {
        const int triangleLength = 10;
        QPolygonF polygon;
        QPointF origin = QPointF(m_position, 0);
        QPointF firstPoint = origin;
        QPointF secondPoint = origin;
        QPointF thirdPoint = origin;
        firstPoint.setX(origin.x() - triangleLength / 2);
        secondPoint.setX(origin.x() + triangleLength / 2);
        thirdPoint.setY(origin.y() + sqrt(3) / 2 * triangleLength);
        polygon << firstPoint << secondPoint << thirdPoint;
        painter->setBrush(Qt::red);
        painter->drawPolygon(polygon);

        polygon.clear();
        origin = QPointF(m_position, m_length);
        firstPoint = origin;
        secondPoint = origin;
        thirdPoint = origin;
        firstPoint.setX(origin.x() - triangleLength / 2);
        secondPoint.setX(origin.x() + triangleLength / 2);
        thirdPoint.setY(origin.y() - sqrt(3) / 2 * triangleLength);
        polygon << firstPoint << secondPoint << thirdPoint;
        painter->setBrush(Qt::red);
        painter->drawPolygon(polygon);
    }
}
