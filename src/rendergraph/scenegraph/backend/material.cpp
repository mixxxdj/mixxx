#include "rendergraph/material.h"

#include "rendergraph/materialshader.h"

using namespace rendergraph::backend;

bool Material::updateUniformsByteArray(QByteArray* buf) {
    auto pThis = static_cast<rendergraph::Material*>(this);
    if (pThis->clearUniformsCacheDirty()) {
        memcpy(buf->data(), pThis->uniformsCache().data(), pThis->uniformsCache().size());
        return true;
    }
    return false;
}

int Material::compare(const QSGMaterial* other) const {
    auto pThis = static_cast<const rendergraph::Material*>(this);
    return pThis->compare(dynamic_cast<const rendergraph::Material*>(other));
}

QSGMaterialShader* Material::createShader(QSGRendererInterface::RenderMode) const {
    // This looks like a leak but it isn't: we pass ownership to Qt. Qt will
    // cache and reuse the shader for all Material of the same type.
    // TODO make sure that RenderMode is always the same.
    auto pThis = static_cast<const rendergraph::Material*>(this);
    auto pShader = pThis->createShader().release();
    return pShader;
}
