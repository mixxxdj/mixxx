#pragma once

#include <QOpenGLShaderProgram>

#include "rendergraph/materialshader.h"

namespace rendergraph {
class Material;
}

class rendergraph::MaterialShader::Impl : private QOpenGLShaderProgram {
  public:
    Impl(MaterialShader* pOwner,
            const char* vertexShaderFile,
            const char* fragmentShaderFile,
            const UniformSet& uniformSet,
            const AttributeSet& attributeSet);

    QOpenGLShaderProgram& glShader() {
        return *this;
    }

    int attributeLocation(int attributeIndex) const {
        return m_attributeLocations[attributeIndex];
    }

    int uniformLocation(int uniformIndex) const {
        return m_uniformLocations[uniformIndex];
    }

    Material* lastModifiedByMaterial() const {
        return m_pLastModifiedByMaterial;
    }
    void setLastModifiedByMaterial(Material* pMaterial) {
        m_pLastModifiedByMaterial = pMaterial;
    }

  private:
    static QString resource(const char* filename) {
        return QString(":/shaders/rendergraph/") + QString(filename) + QString(".gl");
    }
    MaterialShader* const m_pOwner;
    std::vector<int> m_attributeLocations;
    std::vector<int> m_uniformLocations;
    Material* m_pLastModifiedByMaterial{};
};
