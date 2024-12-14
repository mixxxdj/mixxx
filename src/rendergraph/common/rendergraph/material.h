#pragma once

#include <memory>

#include "backend/basematerial.h"
#include "rendergraph/assert.h"
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

    /// See QSGMaterial::compare.
    // Note: QSGMaterial::compare is virtual, so that a concrete Material can
    // implement a custom compare function. But in rendergraph we can always
    // compare the uniforms cache and texture already, and this is sufficient
    // for our purpose.
    int compare(const Material* pOther) const {
        DEBUG_ASSERT(type() == pOther->type());
        int cacheCompareResult = std::memcmp(m_uniformsCache.data(),
                pOther->m_uniformsCache.data(),
                m_uniformsCache.size());
        if (cacheCompareResult != 0) {
            return cacheCompareResult < 0 ? -1 : 1;
        }
        // Note: we currently support only a single texture per material
        if (!texture(0) || !pOther->texture(0)) {
            return texture(0) ? 1 : -1;
        }

        const qint64 diff = texture(0)->comparisonKey() - pOther->texture(0)->comparisonKey();
        return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
    }

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

    virtual Texture* texture(int) const {
        return nullptr;
    }

  private:
    UniformsCache m_uniformsCache;
    bool m_uniformsCacheDirty{};
};
