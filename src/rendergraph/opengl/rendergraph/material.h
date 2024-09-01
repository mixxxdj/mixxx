#pragma once

#include <memory>

#include "rendergraph/uniformscache.h"

namespace rendergraph {
class UniformSet;
class Material;
class MaterialShader;
class MaterialType;
class Texture;
class GeometryNode;
} // namespace rendergraph

class rendergraph::Material {
  public:
    Material(const UniformSet& uniformSet);
    virtual ~Material();
    virtual int compare(const Material* other) const = 0;
    virtual std::unique_ptr<MaterialShader> createShader() const = 0;
    virtual MaterialType* type() const = 0;

    template<typename T>
    void setUniform(int uniformIndex, const T& value) {
        m_uniformsCache.set(uniformIndex, value);
        m_uniformsCacheDirty = true;
    }

    void setShader(std::shared_ptr<MaterialShader> pShader);

    MaterialShader& shader() const;

    int uniformLocation(int uniformIndex) const;
    int attributeLocation(int attributeIndex) const;

  private:
    friend MaterialShader;
    friend GeometryNode;

    void modifyShader();
    bool isLastModifierOfShader() const;

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

    std::shared_ptr<MaterialShader> m_pShader;
    UniformsCache m_uniformsCache;
    bool m_uniformsCacheDirty{};
};
