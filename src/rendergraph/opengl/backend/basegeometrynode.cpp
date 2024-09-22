#include "backend/basegeometrynode.h"

#include <QOpenGLTexture>

#include "backend/shadercache.h"
#include "rendergraph/geometrynode.h"
#include "rendergraph/texture.h"

using namespace rendergraph;

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

void BaseGeometryNode::initializeBackend() {
    initializeOpenGLFunctions();
    GeometryNode* pThis = static_cast<GeometryNode*>(this);
    pThis->material().setShader(ShaderCache::getShaderForMaterial(&pThis->material()));
}

void BaseGeometryNode::renderBackend() {
    GeometryNode* pThis = static_cast<GeometryNode*>(this);
    Geometry& geometry = pThis->geometry();
    Material& material = pThis->material();

    if (geometry.vertexCount() == 0) {
        return;
    }

    glEnable(GL_BLEND);
    // qt scene graph uses premultiplied alpha color in the shader,
    // so we need to do the same
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

    for (int i = 0; i < geometry.attributeCount(); i++) {
        int location = material.attributeLocation(i);
        shader.enableAttributeArray(location);
        shader.setAttributeArray(location,
                geometry.vertexData() + geometry.offset(i),
                geometry.tupleSize(i),
                geometry.stride());
    }

    // TODO multiple textures
    auto pTexture = material.texture(1);
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
