#pragma once

#include <QSGMaterial>

#include "materialshader_impl.h"
#include "materialtype_impl.h"
#include "rendergraph/material.h"
#include "texture_impl.h"

class rendergraph::Material::Impl : public QSGMaterial {
  public:
    Impl(Material* pOwner)
            : m_pOwner(pOwner) {
        setFlag(QSGMaterial::Blending);
    }

    QSGMaterial* sgMaterial() {
        return this;
    }

    bool updateUniformsByteArray(QByteArray* buf) {
        if (m_pOwner->clearUniformsCacheDirty()) {
            memcpy(buf->data(), m_pOwner->uniformsCache().data(), m_pOwner->uniformsCache().size());
            return true;
        }
        return false;
    }

    QSGTexture* texture(int binding) {
        return m_pOwner->texture(binding)->impl().sgTexture();
    }

  private:
    QSGMaterialType* type() const override {
        return m_pOwner->type()->impl().sgMaterialType();
    }

    int compare(const QSGMaterial* other) const override {
        const Impl* otherCasted = static_cast<const Impl*>(other);
        return otherCasted && m_pOwner->compare(otherCasted->m_pOwner);
    }

    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override {
        auto pShader = m_pOwner->createShader().release(); // This leaks
        return pShader->impl().sgMaterialShader();
    }

    Material* m_pOwner;
};
