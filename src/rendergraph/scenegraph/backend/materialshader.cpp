#include "backend/materialshader.h"

#include <QSGTexture>

#include "rendergraph/material.h"

using namespace rendergraph::backend;

bool MaterialShader::updateUniformData(RenderState& state,
        QSGMaterial* newMaterial,
        QSGMaterial* oldMaterial) {
    return static_cast<Material*>(newMaterial)->updateUniformsByteArray(state.uniformData());
}

void MaterialShader::updateSampledImage(RenderState& state,
        int binding,
        QSGTexture** texture,
        QSGMaterial* newMaterial,
        QSGMaterial* oldMaterial) {
    *texture = static_cast<rendergraph::Material*>(newMaterial)->texture(binding)->backendTexture();
    (*texture)->commitTextureOperations(state.rhi(), state.resourceUpdateBatch());
}
