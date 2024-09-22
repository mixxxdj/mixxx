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
    struct Point2D {
        QVector2D position2D;
        Point2D(float x, float y)
                : position2D{x, y} {
        }
    };

    struct TexturedPoint2D {
        QVector2D position2D;
        QVector2D texcoord2D;
        TexturedPoint2D(float x, float y, float tx, float ty)
                : position2D{x, y},
                  texcoord2D{tx, ty} {
        }
    };

    struct RGBColoredPoint2D {
        QVector2D position2D;
        QVector3D color3D;
        RGBColoredPoint2D(float x, float y, float r, float g, float b)
                : position2D{x, y},
                  color3D{r, g, b} {
        }
    };

    struct RGBAColoredPoint2D {
        QVector2D position2D;
        QVector4D color4D;
        RGBAColoredPoint2D(float x, float y, float r, float g, float b, float a)
                : position2D{x, y},
                  color4D{r, g, b, a} {
        }
    };

    enum class DrawingMode {
        Triangles,
        TriangleStrip
    };

    Geometry(const rendergraph::AttributeSet& attributeSet, int vertexCount);
    ~Geometry();

    void allocate(int vertexCount);

    void setAttributeValues(int attributePosition, const float* data, int numTuples);

    float* vertexData();

    template<typename T>
    T* vertexDataAs() {
        return reinterpret_cast<T*>(vertexData());
    }

    DrawingMode drawingMode() const;
    void setDrawingMode(DrawingMode mode);
};
