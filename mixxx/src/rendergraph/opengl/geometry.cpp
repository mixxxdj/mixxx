#include "rendergraph/geometry.h"

#include "rendergraph/assert.h"
#include "rendergraph/attributeset.h"

using namespace rendergraph;

Geometry::Geometry(const AttributeSet& attributeSet, int vertexCount)
        : BaseGeometry(attributeSet, vertexCount) {
}

void Geometry::setAttributeValues(int attributePosition, const float* from, int numTuples) {
    // Note: this code assumes all vertices are floats, which is sufficient for
    // our purpose.
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

void Geometry::setDrawingMode(DrawingMode mode) {
    m_drawingMode = mode;
}

DrawingMode Geometry::drawingMode() const {
    return m_drawingMode;
}
