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
