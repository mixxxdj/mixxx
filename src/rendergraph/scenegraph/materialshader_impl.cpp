#include "materialshader_impl.h"

#include "material_impl.h"

using namespace rendergraph;

bool rendergraph::MaterialShader::Impl::updateUniformData(RenderState& state,
        QSGMaterial* newMaterial,
        QSGMaterial* oldMaterial) {
    Material::Impl* pMaterialImpl = static_cast<Material::Impl*>(newMaterial);
    return pMaterialImpl->updateUniformsByteArray(state.uniformData());
}

void MaterialShader::Impl::updateSampledImage(RenderState& state,
        int binding,
        QSGTexture** texture,
        QSGMaterial* newMaterial,
        QSGMaterial* oldMaterial) {
    Material::Impl* pMaterialImpl = static_cast<Material::Impl*>(newMaterial);
    *texture = pMaterialImpl->texture(binding);
    (*texture)->commitTextureOperations(state.rhi(), state.resourceUpdateBatch());
}
