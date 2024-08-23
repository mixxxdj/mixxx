#include "rendergraph/material.h"

#include "material_impl.h"

using namespace rendergraph;

Material::Material(Impl* pImpl, const UniformSet& uniformSet)
        : m_pImpl(pImpl),
          m_uniformsCache(uniformSet) {
}

Material::Material(const UniformSet& uniformSet)
        : Material(new Material::Impl(this), uniformSet) {
}

Material::~Material() = default;

Material::Impl& Material::impl() const {
    return *m_pImpl;
}
