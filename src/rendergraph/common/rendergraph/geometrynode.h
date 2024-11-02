#pragma once

#include "backend/basegeometrynode.h"
#include "rendergraph/geometry.h"
#include "rendergraph/material.h"
#include "rendergraph/nodeinterface.h"

namespace rendergraph {
class GeometryNode;
} // namespace rendergraph

class rendergraph::GeometryNode : public rendergraph::NodeInterface<rendergraph::BaseGeometryNode> {
  public:
    GeometryNode();
    virtual ~GeometryNode() = default;

    template<class T_Material>
    void initForRectangles(int numRectangles) {
        const int verticesPerRectangle = 6; // 2 triangles
        setGeometry(std::make_unique<Geometry>(T_Material::attributes(),
                numRectangles * verticesPerRectangle));
        setMaterial(std::make_unique<T_Material>());
        geometry().setDrawingMode(DrawingMode::Triangles);
    }

    void setUsePreprocess(bool value);
    void setMaterial(std::unique_ptr<Material> material);
    void setGeometry(std::unique_ptr<Geometry> geometry);

    Geometry& geometry() const;
    Material& material() const;

    void markDirtyGeometry();
    void markDirtyMaterial();

  private:
    std::unique_ptr<Material> m_pMaterial;
    std::unique_ptr<Geometry> m_pGeometry;
};
