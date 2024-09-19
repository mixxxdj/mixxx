#include "backend/geometry.h"

#include "rendergraph/geometry.h"

using namespace rendergraph::backend;

Geometry::Geometry(
        const rendergraph::AttributeSet& attributeSet, int vertexCount)
        : m_drawingMode(static_cast<int>(rendergraph::Geometry::DrawingMode::
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

int Geometry::attributeCount() const {
    return m_tupleSizes.size();
}

int Geometry::vertexCount() const {
    return m_vertexCount;
}

int Geometry::offset(int attributeIndex) const {
    return m_offsets[attributeIndex];
}

int Geometry::tupleSize(int attributeIndex) const {
    return m_tupleSizes[attributeIndex];
}

int Geometry::stride() const {
    return m_stride;
}
