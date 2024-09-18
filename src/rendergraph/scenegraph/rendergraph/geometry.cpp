#include "rendergraph/geometry.h"

#include "rendergraph/attributeset.h"

using namespace rendergraph;

Geometry::Geometry(const rendergraph::AttributeSet& attributeSet, int vertexCount)
        : QSGGeometry(attributeSet.sgAttributeSet(), vertexCount),
          m_stride(attributeSet.sgAttributeSet().stride) {
    QSGGeometry::setDrawingMode(QSGGeometry::DrawTriangleStrip);
}

Geometry::~Geometry() = default;

void Geometry::setAttributeValues(int attributePosition, const float* from, int numTuples) {
    const auto attributeArray = QSGGeometry::attributes();
    int offset = 0;
    for (int i = 0; i < attributePosition; i++) {
        offset += attributeArray[i].tupleSize;
    }
    const int tupleSize = attributeArray[attributePosition].tupleSize;
    const int skip = m_stride / sizeof(float) - tupleSize;

    float* to = static_cast<float*>(QSGGeometry::vertexData());
    to += offset;
    while (numTuples--) {
        int k = tupleSize;
        while (k--) {
            *to++ = *from++;
        }
        to += skip;
    }
}

float* Geometry::vertexData() {
    return static_cast<float*>(QSGGeometry::vertexData());
}

void Geometry::allocate(int vertexCount) {
    QSGGeometry::allocate(vertexCount);
}

void Geometry::setDrawingMode(Geometry::DrawingMode mode) {
    QSGGeometry::setDrawingMode(toSgDrawingMode(mode));
}

Geometry::DrawingMode Geometry::drawingMode() const {
    return fromSgDrawingMode(QSGGeometry::drawingMode());
}

QSGGeometry::DrawingMode Geometry::toSgDrawingMode(Geometry::DrawingMode mode) {
    switch (mode) {
    case Geometry::DrawingMode::Triangles:
        return QSGGeometry::DrawTriangles;
    case Geometry::DrawingMode::TriangleStrip:
        return QSGGeometry::DrawTriangleStrip;
    }
}

Geometry::DrawingMode Geometry::fromSgDrawingMode(unsigned int mode) {
    switch (mode) {
    case QSGGeometry::DrawTriangles:
        return Geometry::DrawingMode::Triangles;
    case QSGGeometry::DrawTriangleStrip:
        return Geometry::DrawingMode::TriangleStrip;
    default:
        throw "not implemented";
    }
}
