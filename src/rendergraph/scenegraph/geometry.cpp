#include "rendergraph/geometry.h"

#include <stdexcept>

#include "rendergraph/assert.h"

using namespace rendergraph;

namespace {
QSGGeometry::DrawingMode toSgDrawingMode(DrawingMode mode) {
    switch (mode) {
    case DrawingMode::Triangles:
        return QSGGeometry::DrawTriangles;
    case DrawingMode::TriangleStrip:
        return QSGGeometry::DrawTriangleStrip;
    default:
        throw std::runtime_error("not implemented");
    }
}

DrawingMode fromSgDrawingMode(unsigned int mode) {
    switch (mode) {
    case QSGGeometry::DrawTriangles:
        return DrawingMode::Triangles;
    case QSGGeometry::DrawTriangleStrip:
        return DrawingMode::TriangleStrip;
    default:
        throw std::runtime_error("not implemented");
    }
}
} // namespace

Geometry::Geometry(const rendergraph::AttributeSet& attributeSet, int vertexCount)
        : BaseGeometry(attributeSet, vertexCount) {
}

void Geometry::setAttributeValues(int attributePosition, const float* from, int numTuples) {
    // Note: this code assumes all vertices are floats, which is sufficient for our
    // purpose.
    const auto attributeArray = QSGGeometry::attributes();
    int vertexOffset = 0;
    for (int i = 0; i < attributePosition; i++) {
        vertexOffset += attributeArray[i].tupleSize;
    }
    const int tupleSize = attributeArray[attributePosition].tupleSize;
    const int vertexStride = sizeOfVertex() / sizeof(float);
    const int vertexSkip = vertexStride - tupleSize;

    VERIFY_OR_DEBUG_ASSERT(vertexOffset + numTuples * vertexStride - vertexSkip <=
            vertexCount() * vertexStride) {
        return;
    }

    float* to = static_cast<float*>(QSGGeometry::vertexData());
    to += vertexOffset;
    while (numTuples--) {
        int k = tupleSize;
        while (k--) {
            *to++ = *from++;
        }
        to += vertexSkip;
    }
}

void Geometry::setDrawingMode(DrawingMode mode) {
    QSGGeometry::setDrawingMode(toSgDrawingMode(mode));
}

DrawingMode Geometry::drawingMode() const {
    return fromSgDrawingMode(QSGGeometry::drawingMode());
}
