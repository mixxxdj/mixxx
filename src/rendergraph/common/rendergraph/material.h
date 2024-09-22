#pragma once

#include <memory>

#include "backend/basematerial.h"
#include "rendergraph/materialshader.h"
#include "rendergraph/materialtype.h"
#include "rendergraph/texture.h"
#include "rendergraph/uniformscache.h"
#include "rendergraph/uniformset.h"

namespace rendergraph {
class Material;
} // namespace rendergraph

class rendergraph::Material : public rendergraph::BaseMaterial {
  public:
    Material(const UniformSet& uniformSet);
    virtual ~Material();
    virtual int compare(const Material* other) const = 0;
    virtual std::unique_ptr<MaterialShader> createShader() const = 0;

    template<typename T>
    void setUniform(int uniformIndex, const T& value) {
        m_uniformsCache.set(uniformIndex, value);
        m_uniformsCacheDirty = true;
    }

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
    UniformsCache m_uniformsCache;
    bool m_uniformsCacheDirty{};
};
