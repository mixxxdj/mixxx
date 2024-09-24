#include "rendergraph/geometry.h"

#include "rendergraph/assert.h"
#include "rendergraph/attributeset.h"

using namespace rendergraph;

Geometry::Geometry(const AttributeSet& attributeSet, int vertexCount)
        : BaseGeometry(attributeSet, vertexCount) {
}

void Geometry::setAttributeValues(int attributePosition, const float* from, int numTuples) {
    // TODO this code assumes all vertices are floats
    VERIFY_OR_DEBUG_ASSERT(attributePosition < attributeCount()) {
        return;
    }
    const int vertexOffset = attributes()[attributePosition].m_offset / sizeof(float);
    const int tupleSize = attributes()[attributePosition].m_tupleSize;
    const int vertexStride = sizeOfVertex() / sizeof(float);
    const int vertexSkip = vertexStride - tupleSize;

    VERIFY_OR_DEBUG_ASSERT(vertexOffset + numTuples * vertexStride - vertexSkip <=
            static_cast<int>(m_vertexData.size())) {
        return;
    }

    float* to = m_vertexData.data();
    to += vertexOffset;

    while (numTuples--) {
        int k = tupleSize;
        while (k--) {
            *to++ = *from++;
        }
        to += vertexSkip;
    }
}

void Geometry::setDrawingMode(Geometry::DrawingMode mode) {
    m_drawingMode = static_cast<int>(mode);
}

Geometry::DrawingMode Geometry::drawingMode() const {
    return static_cast<Geometry::DrawingMode>(m_drawingMode);
}
