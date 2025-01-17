#pragma once

#include <QVector2D>
#include <QVector3D>

#include "backend/basegeometry.h"
#include "rendergraph/attributeset.h"

namespace rendergraph {
class Geometry;
} // namespace rendergraph

class rendergraph::Geometry : public rendergraph::BaseGeometry {
  public:
    using DrawingMode = rendergraph::DrawingMode;

    struct Point2D {
        QVector2D position2D;
    };

    struct TexturedPoint2D {
        QVector2D position2D;
        QVector2D texcoord2D;
    };

    struct RGBColoredPoint2D {
        QVector2D position2D;
        QVector3D color3D;
    };

    struct RGBAColoredPoint2D {
        QVector2D position2D;
        QVector4D color4D;
    };

    Geometry(const rendergraph::AttributeSet& attributeSet, int vertexCount);

    const Attribute* attributes() const {
        return BaseGeometry::attributes();
    }

    void setAttributeValues(int attributePosition, const float* data, int numTuples);

    int attributeCount() const {
        return BaseGeometry::attributeCount();
    }

    void allocate(int vertexCount) {
        BaseGeometry::allocate(vertexCount);
    }

    int sizeOfVertex() const {
        return BaseGeometry::sizeOfVertex();
    }
    int vertexCount() const {
        return BaseGeometry::vertexCount();
    }

    template<typename T>
    T* vertexDataAs() {
        return reinterpret_cast<T*>(vertexData());
    }

    DrawingMode drawingMode() const;

    void setDrawingMode(DrawingMode mode);
};
