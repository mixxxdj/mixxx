#include "backend/material.h"

using namespace rendergraph::backend;

void Material::setShader(std::shared_ptr<rendergraph::MaterialShader> pShader) {
    m_pShader = pShader;
}

rendergraph::MaterialShader& Material::shader() const {
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
