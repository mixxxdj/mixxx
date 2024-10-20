#include "rendergraph/geometrynode.h"

using namespace rendergraph;

GeometryNode::GeometryNode() = default;

void GeometryNode::setUsePreprocess(bool value) {
    BaseNode::setUsePreprocess(value);
}

void GeometryNode::setGeometry(std::unique_ptr<Geometry> pGeometry) {
    m_pGeometry = std::move(pGeometry);
}

void GeometryNode::setMaterial(std::unique_ptr<Material> pMaterial) {
    m_pMaterial = std::move(pMaterial);
}

Geometry& GeometryNode::geometry() const {
    return *m_pGeometry;
}

Material& GeometryNode::material() const {
    return *m_pMaterial;
}

void GeometryNode::markDirtyGeometry() {
    // not (yet) needed for opengl
}

void GeometryNode::markDirtyMaterial() {
    // not (yet) needed for opengl
}
