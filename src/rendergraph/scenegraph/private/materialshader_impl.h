#pragma once

#include <QSGMaterialShader>

#include "rendergraph/materialshader.h"

class rendergraph::MaterialShader::Impl : public QSGMaterialShader {
  public:
    Impl(MaterialShader* pOwner,
            const char* vertexShaderFile,
            const char* fragmentShaderFile,
            const UniformSet& uniformSet,
            const AttributeSet& attributeSet)
            : m_pOwner(pOwner) {
        (void)uniformSet;
        (void)attributeSet;
        setShaderFileName(VertexStage, resource(vertexShaderFile));
        setShaderFileName(FragmentStage, resource(fragmentShaderFile));
    }

    QSGMaterialShader* sgMaterialShader() {
        return this;
    }

  private:
    bool updateUniformData(RenderState& state,
            QSGMaterial* newMaterial,
            QSGMaterial* oldMaterial) override;

    void updateSampledImage(RenderState& state,
            int binding,
            QSGTexture** texture,
            QSGMaterial* newMaterial,
            QSGMaterial* oldMaterial) override;

    static QString resource(const char* filename) {
        return QString(":/shaders/rendergraph/") + QString(filename) + QString(".qsb");
    }

    MaterialShader* m_pOwner;
};
