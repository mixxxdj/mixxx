#pragma once

#include "rendergraph/node.h"

namespace rendergraph {
class GeometryNode;
class Geometry;
class Material;
} // namespace rendergraph

class rendergraph::GeometryNode : public rendergraph::Node {
  public:
    class Impl;

    GeometryNode();
    ~GeometryNode();
    void setMaterial(std::unique_ptr<Material> material);
    void setGeometry(std::unique_ptr<Geometry> geometry);
    Geometry& geometry() const;
    Material& material() const;

  private:
    GeometryNode(NodeImplBase* pImpl);
    std::unique_ptr<Material> m_material;
    std::unique_ptr<Geometry> m_geometry;
};
