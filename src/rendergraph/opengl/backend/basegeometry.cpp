#include "backend/basegeometry.h"

#include "rendergraph/geometry.h"

using namespace rendergraph;

BaseGeometry::BaseGeometry(
        const AttributeSet& attributeSet, int vertexCount)
        : m_drawingMode(static_cast<int>(Geometry::DrawingMode::
                          TriangleStrip)) // to mimic sg default
          ,
          m_vertexCount(vertexCount) {
    int offset = 0;
    for (const auto& attribute : attributeSet.attributes()) {
        m_offsets.push_back(offset);
        offset += attribute.m_tupleSize;
        m_tupleSizes.push_back(attribute.m_tupleSize);
    }
    m_stride = offset * sizeof(float);
    m_data.resize(offset * vertexCount);
}

int BaseGeometry::attributeCount() const {
    return m_tupleSizes.size();
}

int BaseGeometry::vertexCount() const {
    return m_vertexCount;
}

int BaseGeometry::offset(int attributeIndex) const {
    return m_offsets[attributeIndex];
}

int BaseGeometry::tupleSize(int attributeIndex) const {
    return m_tupleSizes[attributeIndex];
}

int BaseGeometry::stride() const {
    return m_stride;
}
