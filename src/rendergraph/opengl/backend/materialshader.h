#pragma once

#include <QOpenGLShaderProgram>
#include <QString>

namespace rendergraph::backend {
class Material;
class MaterialShader;
} // namespace rendergraph::backend

class rendergraph::backend::MaterialShader : public QOpenGLShaderProgram {
  protected:
    MaterialShader() = default;

  public:
    int attributeLocation(int attributeIndex) const;
    int uniformLocation(int uniformIndex) const;

    Material* lastModifiedByMaterial() const;
    void setLastModifiedByMaterial(Material* pMaterial);

    static QString resource(const char* filename) {
        return QString(":/shaders/rendergraph/") + QString(filename) + QString(".gl");
    }
    std::vector<int> m_attributeLocations;
    std::vector<int> m_uniformLocations;
    Material* m_pLastModifiedByMaterial{};
};
