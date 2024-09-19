#include "backend/materialshader.h"

using namespace rendergraph::backend;

int MaterialShader::attributeLocation(int attributeIndex) const {
    return m_attributeLocations[attributeIndex];
}

int MaterialShader::uniformLocation(int uniformIndex) const {
    return m_uniformLocations[uniformIndex];
}

Material* MaterialShader::lastModifiedByMaterial() const {
    return m_pLastModifiedByMaterial;
}
void MaterialShader::setLastModifiedByMaterial(Material* pMaterial) {
    m_pLastModifiedByMaterial = pMaterial;
}
