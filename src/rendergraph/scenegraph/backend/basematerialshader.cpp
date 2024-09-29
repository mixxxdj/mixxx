#include "backend/basematerialshader.h"

#include <QSGTexture>

#include "rendergraph/material.h"

using namespace rendergraph;

bool BaseMaterialShader::updateUniformData(RenderState& state,
        QSGMaterial* newMaterial,
        QSGMaterial* oldMaterial) {
    return static_cast<Material*>(newMaterial)->updateUniformsByteArray(state.uniformData());
}

// override for QSGMaterialShader; this function is called by the Qt scene graph to prepare use of
// sampled images in the shader, typically in the form of combined image samplers.
void BaseMaterialShader::updateSampledImage(RenderState& state,
        int binding,
        QSGTexture** texture,
        QSGMaterial* newMaterial,
        QSGMaterial* oldMaterial) {
    *texture = static_cast<Material*>(newMaterial)->texture(binding)->backendTexture();
    (*texture)->commitTextureOperations(state.rhi(), state.resourceUpdateBatch());
}
