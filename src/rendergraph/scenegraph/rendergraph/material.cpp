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
    // This looks like a leak but it isn't: we pass ownership to Qt. Qt will
    // cache and reuse the shader for all Material of the same type.
    // TODO make sure that RenderMode is always the same.
    auto pShader = createShader().release();
    return pShader;
}
