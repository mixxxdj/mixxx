#include "backend/basematerialshader.h"

using namespace rendergraph;

int BaseMaterialShader::attributeLocation(int attributeIndex) const {
    return m_attributeLocations[attributeIndex];
}

int BaseMaterialShader::uniformLocation(int uniformIndex) const {
    return m_uniformLocations[uniformIndex];
}

BaseMaterial* BaseMaterialShader::lastModifiedByMaterial() const {
    return m_pLastModifiedByMaterial;
}

void BaseMaterialShader::setLastModifiedByMaterial(BaseMaterial* pMaterial) {
    m_pLastModifiedByMaterial = pMaterial;
}
