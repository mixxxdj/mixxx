#pragma once

#include <QSGMaterialShader>

namespace rendergraph {
class AttributeSet;
class UniformSet;
class MaterialShader;
} // namespace rendergraph

class rendergraph::MaterialShader : public QSGMaterialShader {
  public:
    MaterialShader(const char* vertexShaderFile,
            const char* fragmentShaderFile,
            const UniformSet& uniforms,
            const AttributeSet& attributeSet);
    ~MaterialShader();

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
};
