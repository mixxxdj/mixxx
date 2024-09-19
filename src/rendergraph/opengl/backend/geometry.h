#pragma once

#include <vector>

#include "rendergraph/attributeset.h"

namespace rendergraph::backend {
class Geometry;
}

class rendergraph::backend::Geometry {
  protected:
    Geometry(const rendergraph::AttributeSet& attributeSet, int vertexCount);

  public:
    int attributeCount() const;
    int vertexCount() const;
    int offset(int attributeIndex) const;
    int tupleSize(int attributeIndex) const;
    int stride() const;

  protected:
    int m_drawingMode;
    int m_vertexCount;
    std::vector<int> m_tupleSizes;
    std::vector<int> m_offsets;
    int m_stride;
    std::vector<float> m_data;
};
