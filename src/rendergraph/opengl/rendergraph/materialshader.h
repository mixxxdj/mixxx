#pragma once

#include <QOpenGLShaderProgram>
#include <QString>

namespace rendergraph {
class AttributeSet;
class UniformSet;
class MaterialShader;
class Material;
} // namespace rendergraph

class rendergraph::MaterialShader : public QOpenGLShaderProgram {
  public:
    MaterialShader(const char* vertexShaderFile,
            const char* fragmentShaderFile,
            const UniformSet& uniforms,
            const AttributeSet& attributeSet);
    ~MaterialShader();

    QOpenGLShaderProgram& glShader();
    int attributeLocation(int attributeIndex) const;
    int uniformLocation(int uniformIndex) const;

  private:
    friend Material;

    Material* lastModifiedByMaterial() const;
    void setLastModifiedByMaterial(Material* pMaterial);

    static QString resource(const char* filename) {
        return QString(":/shaders/rendergraph/") + QString(filename) + QString(".gl");
    }
    std::vector<int> m_attributeLocations;
    std::vector<int> m_uniformLocations;
    Material* m_pLastModifiedByMaterial{};
};
