#pragma once

#include <QSGMaterial>
#include <memory>

#include "rendergraph/materialtype.h"
#include "rendergraph/texture.h"
#include "rendergraph/uniformscache.h"

namespace rendergraph {
class UniformSet;
class Material;
class MaterialShader;
class MaterialType;
class Texture;
} // namespace rendergraph

class rendergraph::Material : public QSGMaterial {
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

    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    int compare(const QSGMaterial* other) const override {
        return compare(dynamic_cast<const Material*>(other));
    }

    bool updateUniformsByteArray(QByteArray* buf);

  private:
    UniformsCache m_uniformsCache;
    bool m_uniformsCacheDirty{};
};
