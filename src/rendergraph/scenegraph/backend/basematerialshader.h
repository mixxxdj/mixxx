#pragma once

#include <QSGMaterialShader>
#include <QString>

namespace rendergraph {
class BaseMaterialShader;
} // namespace rendergraph

class rendergraph::BaseMaterialShader : public QSGMaterialShader {
  protected:
    BaseMaterialShader() = default;

  public:
    bool updateUniformData(RenderState& state,
            QSGMaterial* newMaterial,
            QSGMaterial* oldMaterial) override;

    void updateSampledImage(RenderState& state,
            int binding,
            QSGTexture** texture,
            QSGMaterial* newMaterial,
            QSGMaterial* oldMaterial) override;
};
