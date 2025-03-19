#pragma once

#include <QString>
#include <vector>

#include "rendergraph/types.h"

namespace rendergraph {
class BaseAttributeSet; // fwd decl to avoid circular dependency
class BaseGeometry;
} // namespace rendergraph

// Note: this assumes all vertices consist of floats, which is sufficient for our purposes
class rendergraph::BaseGeometry {
  protected:
    BaseGeometry(const BaseAttributeSet& attributeSet, int vertexCount);

  public:
    struct Attribute {
        const int m_offset;
        const int m_tupleSize;
        const PrimitiveType m_primitiveType;
        const QString m_name;
    };

    float* vertexData() {
        return m_vertexData.data();
    }
    const float* vertexData() const {
        return m_vertexData.data();
    }
    const Attribute* attributes() const {
        return m_pAttributes;
    }
    int attributeCount() const {
        return m_attributeCount;
    }
    int vertexCount() const {
        return m_vertexCount;
    }
    int sizeOfVertex() const { // in bytes
        return m_sizeOfVertex;
    }
    void allocate(int vertexCount) {
        if (m_vertexCount == vertexCount) {
            return;
        }
        m_vertexCount = vertexCount;
        m_vertexData.resize(m_vertexCount * sizeOfVertex() / sizeof(float));
    }

  protected:
    const Attribute* m_pAttributes;
    const int m_attributeCount;
    const int m_sizeOfVertex;
    DrawingMode m_drawingMode;
    int m_vertexCount;
    std::vector<float> m_vertexData;
};
