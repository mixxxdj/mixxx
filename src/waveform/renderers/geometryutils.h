#pragma once

#include <QPainter>

enum class Direction : int {
    UP = 0,
    DOWN,
    LEFT,
    RIGHT
};

/// Generate an equilateral triangle as a QPolygonF
/// @param edgeLength The edge length of the equilateral triangle.
/// @param baseMidPoint The coordinates of the mid point of the triangle base in drawing space.
/// @param pointingDirection The direction the triangle faces with respect to the base mid point.
/// @return Equilateral triangle as QPolygonF
QPolygonF getEquilateralTriangle(float edgeLength,
        QPointF baseMidPoint,
        Direction pointingDirection);
