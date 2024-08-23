#pragma once

#include "attributeset_impl.h"
#include "rendergraph/geometry.h"

class rendergraph::Geometry::Impl {
  public:
    Impl(const AttributeSet& attributeSet, int vertexCount)
            : m_drawingMode(Geometry::DrawingMode::TriangleStrip) // to mimic sg default
              ,
              m_vertexCount(vertexCount) {
        for (const auto& attribute : attributeSet.impl().attributes()) {
            m_data.emplace_back(std::vector<float>(m_vertexCount * attribute.m_tupleSize));
            m_tupleSizes.push_back(attribute.m_tupleSize);
        }
    }

    int attributeCount() const {
        return m_tupleSizes.size();
    }

    int vertexCount() const {
        return m_vertexCount;
    }

    float const* vertexData(int attributeIndex) const {
        return m_data[attributeIndex].data();
    }

    int tupleSize(int attributeIndex) const {
        return m_tupleSizes[attributeIndex];
    }

    void setAttributeValues(int attributePosition, const float* from, int numTuples) {
        memcpy(m_data[attributePosition].data(),
                from,
                numTuples * m_tupleSizes[attributePosition] * sizeof(float));
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
    std::vector<std::vector<float>> m_data;
};
