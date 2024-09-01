#include "rendergraph/geometry.h"

#include "rendergraph/attributeset.h"

using namespace rendergraph;

Geometry::Geometry(const AttributeSet& attributeSet, int vertexCount)
        : m_drawingMode(Geometry::DrawingMode::TriangleStrip) // to mimic sg default
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

template<typename T>
T* Geometry::vertexDataAs() {
    return reinterpret_cast<T*>(vertexData());
}

template Geometry::Point2D* Geometry::vertexDataAs<Geometry::Point2D>();

template Geometry::TexturedPoint2D* Geometry::vertexDataAs<Geometry::TexturedPoint2D>();

template Geometry::RGBColoredPoint2D* Geometry::vertexDataAs<Geometry::RGBColoredPoint2D>();

template Geometry::RGBAColoredPoint2D* Geometry::vertexDataAs<Geometry::RGBAColoredPoint2D>();

void Geometry::allocate(int vertexCount) {
    m_vertexCount = vertexCount;
    m_data.resize((m_stride / sizeof(float)) * m_vertexCount);
}

void Geometry::setDrawingMode(Geometry::DrawingMode mode) {
    m_drawingMode = mode;
}

Geometry::DrawingMode Geometry::drawingMode() const {
    return m_drawingMode;
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

Geometry::DrawingMode m_drawingMode;
int m_vertexCount;
std::vector<int> m_tupleSizes;
std::vector<int> m_offsets;
int m_stride;
std::vector<float> m_data;
