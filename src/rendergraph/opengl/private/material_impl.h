#pragma once

#include "materialshader_impl.h"
#include "rendergraph/material.h"
#include "texture_impl.h"

class rendergraph::Material::Impl {
  public:
    Impl(Material* pOwner)
            : m_pOwner(pOwner) {
    }

    void setShader(MaterialShader* pShader) {
        m_pShader = pShader;
    }

    int attributeLocation(int attributeIndex) const {
        return m_pShader->impl().attributeLocation(attributeIndex);
    }

    QOpenGLTexture* texture(int binding) {
        return m_pOwner->texture(binding)->impl().glTexture();
    }

    int uniformLocation(int uniformIndex) const {
        return m_pShader->impl().uniformLocation(uniformIndex);
    }

    QOpenGLShaderProgram& glShader() const {
        return m_pShader->impl().glShader();
    }

  private:
    MaterialShader* m_pShader{};
    Material* const m_pOwner;
};
