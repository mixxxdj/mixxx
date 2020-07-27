#pragma once

#include <QPainter>

#include "util/math.h"

namespace {
inline float getEquilateralTriangleAltitude(float triangleEdgeLength) {
    return sqrt(3) / 2 * triangleEdgeLength;
}
} // namespace

enum class Direction : int {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT
};

/**
 * Generate an equilateral triangle as a QPolygonF
 * @param edgeLength The edge length of the equilateral triangle.
 * @param baseMidPoint The coordinates of the mid point of the triangle base in drawing space.
 * @param pointingDirection The direction the triangle faces with respect to the base mid point.
 * @return Equilateral triangle as QPolygonF
 */
inline QPolygonF getEquilateralTriangle(float edgeLength,
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