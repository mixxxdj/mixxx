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

    static QString resource(const QString& filename) {
        return QString(":/shaders/rendergraph/%1.gl").arg(filename);
    }
    std::vector<int> m_attributeLocations;
    std::vector<int> m_uniformLocations;
    BaseMaterial* m_pLastModifiedByMaterial{};
};
