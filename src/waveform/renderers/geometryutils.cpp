#include "waveform/renderers/geometryutils.h"

#include "util/math.h"

namespace {
inline float getEquilateralTriangleAltitude(float triangleEdgeLength) {
    return sqrtf(3) / 2 * triangleEdgeLength;
}
} // namespace

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
