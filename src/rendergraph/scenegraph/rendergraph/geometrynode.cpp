#include "rendergraph/geometrynode.h"

#include "rendergraph/material.h"

using namespace rendergraph;

GeometryNode::GeometryNode()
        : Node(this) {
}

GeometryNode::~GeometryNode() = default;

void GeometryNode::setGeometry(std::unique_ptr<Geometry> geometry) {
    m_geometry = std::move(geometry);
    QSGGeometryNode::setGeometry(m_geometry.get());
}

void GeometryNode::setMaterial(std::unique_ptr<Material> material) {
    m_material = std::move(material);
    QSGGeometryNode::setMaterial(m_material.get());
}

Geometry& GeometryNode::geometry() const {
    return *m_geometry;
}

Material& GeometryNode::material() const {
    return *m_material;
}
