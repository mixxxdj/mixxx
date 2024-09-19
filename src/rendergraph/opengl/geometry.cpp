#include "rendergraph/geometry.h"

#include "rendergraph/attributeset.h"

using namespace rendergraph;

Geometry::Geometry(const AttributeSet& attributeSet, int vertexCount)
        : backend::Geometry(attributeSet, vertexCount) {
}

Geometry::~Geometry() = default;

void Geometry::setAttributeValues(int attributePosition, const float* from, int numTuples) {
    const int offset = m_offsets[attributePosition];
    const int tupleSize = m_tupleSizes[attributePosition];
    const int skip = m_stride / sizeof(float) - tupleSize;

    float* to = m_data.data();
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
    return m_data.data();
}

void Geometry::allocate(int vertexCount) {
    m_vertexCount = vertexCount;
    m_data.resize((m_stride / sizeof(float)) * m_vertexCount);
}

void Geometry::setDrawingMode(Geometry::DrawingMode mode) {
    m_drawingMode = static_cast<int>(mode);
}

Geometry::DrawingMode Geometry::drawingMode() const {
    return static_cast<Geometry::DrawingMode>(m_drawingMode);
}
