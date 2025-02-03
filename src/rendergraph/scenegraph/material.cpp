#include "rendergraph/material.h"

using namespace rendergraph;

Material::Material(const UniformSet& uniformSet)
        : m_uniformsCache(uniformSet) {
    setFlag(QSGMaterial::Blending);
}

Material::~Material() = default;
