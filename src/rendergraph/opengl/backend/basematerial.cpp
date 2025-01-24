#include "backend/basematerial.h"

#include "rendergraph/material.h"

using namespace rendergraph;

void BaseMaterial::setShader(std::shared_ptr<MaterialShader> pShader) {
    m_pShader = pShader;
}

MaterialShader& BaseMaterial::shader() const {
    return *m_pShader;
}

int BaseMaterial::uniformLocation(int uniformIndex) const {
    return m_pShader->uniformLocation(uniformIndex);
}

int BaseMaterial::attributeLocation(int attributeIndex) const {
    return m_pShader->attributeLocation(attributeIndex);
}

void BaseMaterial::modifyShader() {
    m_pShader->setLastModifiedByMaterial(this);
}

bool BaseMaterial::isLastModifierOfShader() const {
    return this == m_pShader->lastModifiedByMaterial();
}

int BaseMaterial::compare(const BaseMaterial* other) const {
    auto pThis = static_cast<const Material*>(this);
    return pThis->compare(static_cast<const Material*>(other));
}
