#include "rendergraph/material.h"

#include "rendergraph/shadercache.h"
#include "rendergraph/texture.h"

using namespace rendergraph;

Material::Material(const UniformSet& uniformSet)
        : m_uniformsCache(uniformSet) {
}

Material::~Material() = default;

void Material::setShader(std::shared_ptr<MaterialShader> pShader) {
    m_pShader = pShader;
}

MaterialShader& Material::shader() const {
    return *m_pShader;
}

int Material::uniformLocation(int uniformIndex) const {
    return m_pShader->uniformLocation(uniformIndex);
}

int Material::attributeLocation(int attributeIndex) const {
    return m_pShader->attributeLocation(attributeIndex);
}

void Material::modifyShader() {
    m_pShader->setLastModifiedByMaterial(this);
}

bool Material::isLastModifierOfShader() const {
    return this == m_pShader->lastModifiedByMaterial();
}
