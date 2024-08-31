#pragma once

#include "materialshader_impl.h"
#include "rendergraph/material.h"
#include "rendergraph/opengl/shadercache.h"
#include "texture_impl.h"

class rendergraph::Material::Impl {
  public:
    Impl(Material* pOwner)
            : m_pOwner(pOwner) {
    }

    ~Impl() {
        m_pShader.reset();
    }

    void setShader(std::shared_ptr<MaterialShader> pShader) {
        m_pShader = pShader;
    }

    int attributeLocation(int attributeIndex) const {
        return m_pShader->impl().attributeLocation(attributeIndex);
    }

    QOpenGLTexture* texture(int binding) {
        return m_pOwner->texture(binding)->impl().glTexture();
    }

    void modifyShader() {
        m_pShader->impl().setLastModifiedByMaterial(m_pOwner);
    }

    bool isLastModifierOfShader() const {
        return m_pOwner == m_pShader->impl().lastModifiedByMaterial();
    }

    int uniformLocation(int uniformIndex) const {
        return m_pShader->impl().uniformLocation(uniformIndex);
    }

    QOpenGLShaderProgram& glShader() const {
        return m_pShader->impl().glShader();
    }

  private:
    std::shared_ptr<MaterialShader> m_pShader;
    Material* const m_pOwner;
};
