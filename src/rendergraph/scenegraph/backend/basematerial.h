#pragma once

#include <QSGMaterial>

namespace rendergraph {
class BaseMaterial;
} // namespace rendergraph

class rendergraph::BaseMaterial : public QSGMaterial {
  protected:
    BaseMaterial() = default;

  public:
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    int compare(const QSGMaterial* other) const override;

    bool updateUniformsByteArray(QByteArray* buf);
};
