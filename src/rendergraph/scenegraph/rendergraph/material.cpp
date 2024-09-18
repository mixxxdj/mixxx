#include "rendergraph/material.h"

#include "rendergraph/materialshader.h"

using namespace rendergraph;

Material::Material(const UniformSet& uniformSet)
        : m_uniformsCache(uniformSet) {
    setFlag(QSGMaterial::Blending);
}

Material::~Material() = default;

bool Material::updateUniformsByteArray(QByteArray* buf) {
    if (clearUniformsCacheDirty()) {
        memcpy(buf->data(), uniformsCache().data(), uniformsCache().size());
        return true;
    }
    return false;
}

QSGMaterialShader* Material::createShader(QSGRendererInterface::RenderMode) const {
    auto pShader = createShader().release(); // This leaks
    return pShader;
}
