#include "rendergraph/geometrynode.h"

#include "geometrynode_impl.h"
#include "rendergraph/geometry.h"

using namespace rendergraph;

GeometryNode::GeometryNode(NodeImplBase* pImpl)
        : Node(pImpl) {
}

GeometryNode::GeometryNode()
        : GeometryNode(new GeometryNode::Impl(this)) {
}

GeometryNode::~GeometryNode() = default;

void GeometryNode::setGeometry(std::unique_ptr<Geometry> geometry) {
    m_geometry = std::move(geometry);
    static_cast<GeometryNode::Impl&>(impl()).setGeometry(m_geometry.get());
}

void GeometryNode::setMaterial(std::unique_ptr<Material> material) {
    m_material = std::move(material);
    static_cast<GeometryNode::Impl&>(impl()).setMaterial(m_material.get());
}

Geometry& GeometryNode::geometry() const {
    return *m_geometry;
}

Material& GeometryNode::material() const {
    return *m_material;
}
