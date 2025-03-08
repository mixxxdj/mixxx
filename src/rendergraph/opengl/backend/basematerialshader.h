#pragma once

#include <QOpenGLShaderProgram>
#include <QString>

namespace rendergraph {
class BaseMaterial; // fwd decl to avoid circular dependency
class BaseMaterialShader;
} // namespace rendergraph

class rendergraph::BaseMaterialShader : public QOpenGLShaderProgram {
  protected:
    BaseMaterialShader() = default;

  public:
    int attributeLocation(int attributeIndex) const;
    int uniformLocation(int uniformIndex) const;

    BaseMaterial* lastModifiedByMaterial() const;
    void setLastModifiedByMaterial(BaseMaterial* pMaterial);

  protected:
    std::vector<int> m_attributeLocations;
    std::vector<int> m_uniformLocations;
    BaseMaterial* m_pLastModifiedByMaterial{};
};
