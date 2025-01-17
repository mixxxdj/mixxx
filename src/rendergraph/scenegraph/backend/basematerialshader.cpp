#include "backend/basematerialshader.h"

#include <QSGTexture>

#include "rendergraph/material.h"

using namespace rendergraph;

bool BaseMaterialShader::updateUniformData(RenderState& state,
        QSGMaterial* newMaterial,
        QSGMaterial* oldMaterial) {
    bool result = static_cast<Material*>(newMaterial)->updateUniformsByteArray(state.uniformData());
    QByteArray* buf = state.uniformData();

    if (state.isMatrixDirty()) {
        const QMatrix4x4 m = state.combinedMatrix();
        memcpy(buf->data(), m.constData(), 64);
        result = true;
    }

    return result;
}

/// Override for QSGMaterialShader; this function is called by the Qt scene graph to prepare use of
/// sampled images in the shader, typically in the form of combined image samplers.
void BaseMaterialShader::updateSampledImage(RenderState& state,
        int binding,
        QSGTexture** texture,
        QSGMaterial* newMaterial,
        QSGMaterial* oldMaterial) {
    if (!newMaterial || !static_cast<Material*>(newMaterial)->texture(binding)) {
        *texture = nullptr;
        return;
    }
    *texture = static_cast<Material*>(newMaterial)->texture(binding)->backendTexture();
    (*texture)->commitTextureOperations(state.rhi(), state.resourceUpdateBatch());
}
