#include "backend/basegeometrynode.h"

#include <QOpenGLTexture>
#include <stdexcept>

#include "backend/shadercache.h"
#include "rendergraph/engine.h"
#include "rendergraph/geometrynode.h"
#include "rendergraph/texture.h"

using namespace rendergraph;

namespace {
GLenum toGlDrawingMode(DrawingMode mode) {
    switch (mode) {
    case DrawingMode::Triangles:
        return GL_TRIANGLES;
    case DrawingMode::TriangleStrip:
        return GL_TRIANGLE_STRIP;
    default:
        throw std::runtime_error("not implemented");
    }
}
} // namespace

void BaseGeometryNode::initialize() {
    initializeOpenGLFunctions();
    GeometryNode* pThis = static_cast<GeometryNode*>(this);
    pThis->material().setShader(ShaderCache::getShaderForMaterial(&pThis->material()));
    pThis->material().setUniform(0, engine()->matrix());
}

void BaseGeometryNode::render() {
    GeometryNode* pThis = static_cast<GeometryNode*>(this);
    Geometry& geometry = pThis->geometry();
    Material& material = pThis->material();

    if (geometry.vertexCount() == 0) {
        return;
    }

    glEnable(GL_BLEND);
    // Note: Qt scenegraph uses premultiplied alpha color in the shader,
    // so we need to do the same.
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    QOpenGLShaderProgram& shader = material.shader();
    shader.bind();

    if (material.clearUniformsCacheDirty() || !material.isLastModifierOfShader()) {
        material.modifyShader();
        const UniformsCache& cache = material.uniformsCache();
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

    // TODO this code assumes all vertices are floats
    int vertexOffset = 0;
    for (int i = 0; i < geometry.attributeCount(); i++) {
        const Geometry::Attribute& attribute = geometry.attributes()[i];
        int location = material.attributeLocation(i);
        shader.enableAttributeArray(location);
        shader.setAttributeArray(location,
                geometry.vertexDataAs<float>() + vertexOffset,
                attribute.m_tupleSize,
                geometry.sizeOfVertex());
        vertexOffset += attribute.m_tupleSize;
    }

    // TODO multiple textures
    auto* pTexture = material.texture(1);
    if (pTexture) {
        pTexture->backendTexture()->bind();
    }

    glDrawArrays(toGlDrawingMode(geometry.drawingMode()), 0, geometry.vertexCount());

    if (pTexture) {
        pTexture->backendTexture()->release();
    }

    for (int i = 0; i < geometry.attributeCount(); i++) {
        int location = material.attributeLocation(i);
        shader.disableAttributeArray(location);
    }

    shader.release();
}

void BaseGeometryNode::resize(int, int) {
    VERIFY_OR_DEBUG_ASSERT(engine() != nullptr) {
        return;
    }
    GeometryNode* pThis = static_cast<GeometryNode*>(this);
    pThis->material().setUniform(0, engine()->matrix());
}
