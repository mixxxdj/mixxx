#pragma once

#include "geometry_impl.h"
#include "material_impl.h"
#include "node_impl.h"
#include "rendergraph/geometrynode.h"

class rendergraph::GeometryNode::Impl : public QSGGeometryNode, public rendergraph::NodeImplBase {
  public:
    Impl(GeometryNode* pOwner)
            : NodeImplBase(pOwner) {
    }
    QSGNode* sgNode() override {
        return this;
    }
    void setGeometry(Geometry* geometry) {
        QSGGeometryNode::setGeometry(geometry->impl().sgGeometry());
    }
    void setMaterial(Material* material) {
        QSGGeometryNode::setMaterial(material->impl().sgMaterial());
    }
    void preprocess() override {
        owner()->preprocess();
    }
    bool isSubtreeBlocked() const override {
        return owner()->isSubtreeBlocked();
    }
};
