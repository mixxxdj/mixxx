#pragma once

#include "attributeset_impl.h"
#include "rendergraph/geometry.h"

class rendergraph::Geometry::Impl {
  public:
    Impl(const AttributeSet& attributeSet, int vertexCount)
            : m_drawingMode(Geometry::DrawingMode::TriangleStrip) // to mimic sg default
              ,
              m_vertexCount(vertexCount) {
        int offset = 0;
        for (const auto& attribute : attributeSet.impl().attributes()) {
            m_offsets.push_back(offset);
            offset += attribute.m_tupleSize;
            m_tupleSizes.push_back(attribute.m_tupleSize);
        }
        m_stride = offset * sizeof(float);
        m_data.resize(offset * vertexCount);
    }

    void allocate(int vertexCount) {
        m_vertexCount = vertexCount;
        m_data.resize((m_stride / sizeof(float)) * m_vertexCount);
    }

    int attributeCount() const {
        return m_tupleSizes.size();
    }

    int vertexCount() const {
        return m_vertexCount;
    }

    float* vertexData() {
        return m_data.data();
    }

    template<typename T>
    T* vertexDataAs() {
        return reinterpret_cast<T*>(vertexData());
    }

    int offset(int attributeIndex) const {
        return m_offsets[attributeIndex];
    }

    int tupleSize(int attributeIndex) const {
        return m_tupleSizes[attributeIndex];
    }

    int stride() const {
        return m_stride;
    }

    void setAttributeValues(int attributePosition, const float* from, int numTuples) {
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

    void setDrawingMode(Geometry::DrawingMode mode) {
        m_drawingMode = mode;
    }

    Geometry::DrawingMode drawingMode() const {
        return m_drawingMode;
    }

  private:
    Geometry::DrawingMode m_drawingMode;
    int m_vertexCount;
    std::vector<int> m_tupleSizes;
    std::vector<int> m_offsets;
    int m_stride;
    std::vector<float> m_data;
};
