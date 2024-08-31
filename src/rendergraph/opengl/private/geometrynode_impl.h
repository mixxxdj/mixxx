#pragma once

#include <QOpenGLFunctions>

#include "material_impl.h"
#include "node_impl.h"
#include "rendergraph/geometrynode.h"
#include "rendergraph/opengl/shadercache.h"

class rendergraph::GeometryNode::Impl : public rendergraph::Node::Impl, public QOpenGLFunctions {
  public:
    Impl(GeometryNode* pOwner)
            : Node::Impl(pOwner) {
    }

    void setGeometry(Geometry* geometry) {
        m_pGeometry = geometry;
    }

    void setMaterial(Material* material) {
        m_pMaterial = material;
    }

    void initialize() override {
        initializeOpenGLFunctions();
        m_pMaterial->impl().setShader(ShaderCache::getShaderForMaterial(m_pMaterial));
        Node::Impl::initialize();
    }

    void render() override;

  private:
    Geometry* m_pGeometry{};
    Material* m_pMaterial{};
};
