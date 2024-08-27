#pragma once

#include <memory>

namespace rendergraph {
class Geometry;
class AttributeSet;
} // namespace rendergraph

class rendergraph::Geometry {
  public:
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
