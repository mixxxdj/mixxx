#pragma once

#include <QSGMaterial>

namespace rendergraph::backend {
class Material;
}

class rendergraph::backend::Material : public QSGMaterial {
  protected:
    Material() = default;

  public:
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    int compare(const QSGMaterial* other) const override;

    bool updateUniformsByteArray(QByteArray* buf);
};
