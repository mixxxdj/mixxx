#pragma once

#include <QSGMaterialShader>
#include <QString>

namespace rendergraph::backend {
class MaterialShader;
}

class rendergraph::backend::MaterialShader : public QSGMaterialShader {
  protected:
    MaterialShader() = default;

  public:
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
