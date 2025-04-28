#include "rendergraph/material.h"

using namespace rendergraph;

Material::Material(const UniformSet& uniformSet)
        : m_uniformsCache(uniformSet) {
}

Material::~Material() = default;
