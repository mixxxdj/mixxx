#pragma once

#include <QVector2D>
#include <memory>

namespace rendergraph {
class Geometry;
class AttributeSet;
} // namespace rendergraph

class rendergraph::Geometry {
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
    class Impl;

    Geometry(const AttributeSet& attributeSet, int vertexCount);
    ~Geometry();

    void setAttributeValues(int attributePosition, const float* data, int numTuples);
    Impl& impl() const;

    float* vertexData();

    template<typename T>
    T* vertexDataAs();

    void allocate(int vertexCount);

    DrawingMode drawingMode() const;
    void setDrawingMode(DrawingMode mode);

  private:
    Geometry(Impl* pImpl);

    const std::unique_ptr<Impl> m_pImpl;
};
