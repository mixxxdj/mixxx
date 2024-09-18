#include "rendergraph/materialshader.h"

#include <QSGTexture>

#include "rendergraph/material.h"

using namespace rendergraph;

MaterialShader::MaterialShader(const char* vertexShaderFile,
        const char* fragmentShaderFile,
        const UniformSet& uniformSet,
        const AttributeSet& attributeSet) {
    (void)uniformSet;
    (void)attributeSet;
    setShaderFileName(VertexStage, resource(vertexShaderFile));
    setShaderFileName(FragmentStage, resource(fragmentShaderFile));
}

MaterialShader::~MaterialShader() = default;

bool rendergraph::MaterialShader::updateUniformData(RenderState& state,
        QSGMaterial* newMaterial,
        QSGMaterial* oldMaterial) {
    return static_cast<Material*>(newMaterial)->updateUniformsByteArray(state.uniformData());
}

void MaterialShader::updateSampledImage(RenderState& state,
        int binding,
        QSGTexture** texture,
        QSGMaterial* newMaterial,
        QSGMaterial* oldMaterial) {
    *texture = static_cast<Material*>(newMaterial)->texture(binding)->sgTexture();
    (*texture)->commitTextureOperations(state.rhi(), state.resourceUpdateBatch());
}
