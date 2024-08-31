#pragma once

#include <memory>

#include "rendergraph/uniformscache.h"

namespace rendergraph {
class UniformSet;
class Material;
class MaterialShader;
class MaterialType;
class Texture;
} // namespace rendergraph

class rendergraph::Material {
  public:
    class Impl;

    Material(const UniformSet& uniformSet);
    virtual ~Material();
    virtual int compare(const Material* other) const = 0;
    virtual std::shared_ptr<MaterialShader> createShader() const = 0;
    virtual MaterialType* type() const = 0;

    template<typename T>
    void setUniform(int uniformIndex, const T& value) {
        m_uniformsCache.set(uniformIndex, value);
        m_uniformsCacheDirty = true;
    }

    Impl& impl() const;
    const UniformsCache& uniformsCache() const {
        return m_uniformsCache;
    }

    bool clearUniformsCacheDirty() {
        if (m_uniformsCacheDirty) {
            m_uniformsCacheDirty = false;
            return true;
        }
        return false;
    }

    virtual Texture* texture(int /*binding*/) const {
        return nullptr;
    }

  private:
    Material(Impl* pImpl, const UniformSet& uniformSet);

    const std::unique_ptr<Impl> m_pImpl;
    UniformsCache m_uniformsCache;
    bool m_uniformsCacheDirty{};
};
