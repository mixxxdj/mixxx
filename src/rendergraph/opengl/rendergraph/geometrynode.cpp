#include "rendergraph/geometrynode.h"

#include <QOpenGLTexture>

#include "rendergraph/shadercache.h"
#include "rendergraph/texture.h"

using namespace rendergraph;

GeometryNode::GeometryNode() = default;

GeometryNode::~GeometryNode() = default;

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

void GeometryNode::initialize() {
    initializeOpenGLFunctions();
    m_pMaterial->setShader(ShaderCache::getShaderForMaterial(m_pMaterial.get()));
    Node::initialize();
}

namespace {
GLenum toGlDrawingMode(Geometry::DrawingMode mode) {
    switch (mode) {
    case Geometry::DrawingMode::Triangles:
        return GL_TRIANGLES;
    case Geometry::DrawingMode::TriangleStrip:
        return GL_TRIANGLE_STRIP;
    }
}
} // namespace

void GeometryNode::render() {
    Geometry& geometry = *m_pGeometry;
    Material& material = *m_pMaterial;

    if (geometry.vertexCount() == 0) {
        return;
    }

    glEnable(GL_BLEND);
    // qt scene graph uses premultiplied alpha color in the shader,
    // so we need to do the same
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    QOpenGLShaderProgram& shader = material.shader();
    shader.bind();

    if (m_pMaterial->clearUniformsCacheDirty() || !material.isLastModifierOfShader()) {
        material.modifyShader();
        const UniformsCache& cache = m_pMaterial->uniformsCache();
        for (int i = 0; i < cache.count(); i++) {
            int location = material.uniformLocation(i);
            switch (cache.type(i)) {
            case Type::UInt:
                shader.setUniformValue(location, cache.get<GLuint>(i));
                break;
            case Type::Float:
                shader.setUniformValue(location, cache.get<GLfloat>(i));
                break;
            case Type::Vector2D:
                shader.setUniformValue(location, cache.get<QVector2D>(i));
                break;
            case Type::Vector3D:
                shader.setUniformValue(location, cache.get<QVector3D>(i));
                break;
            case Type::Vector4D:
                shader.setUniformValue(location, cache.get<QVector4D>(i));
                break;
            case Type::Matrix4x4:
                shader.setUniformValue(location, cache.get<QMatrix4x4>(i));
                break;
            }
        }
    }

    for (int i = 0; i < geometry.attributeCount(); i++) {
        int location = material.attributeLocation(i);
        shader.enableAttributeArray(location);
        shader.setAttributeArray(location,
                geometry.vertexData() + geometry.offset(i),
                geometry.tupleSize(i),
                geometry.stride());
    }

    // TODO multiple textures
    auto pTexture = m_pMaterial->texture(1);
    if (pTexture) {
        pTexture->glTexture()->bind();
    }

    glDrawArrays(toGlDrawingMode(geometry.drawingMode()), 0, geometry.vertexCount());

    if (pTexture) {
        pTexture->glTexture()->release();
    }

    for (int i = 0; i < geometry.attributeCount(); i++) {
        int location = material.attributeLocation(i);
        shader.disableAttributeArray(location);
    }

    shader.release();

    Node::render();
}
