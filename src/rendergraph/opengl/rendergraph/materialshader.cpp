#include "rendergraph/materialshader.h"

#include <QOpenGLShaderProgram>

#include "rendergraph/attributeset.h"
#include "rendergraph/uniformset.h"
using namespace rendergraph;

MaterialShader::MaterialShader(const char* vertexShaderFile,
        const char* fragmentShaderFile,
        const UniformSet& uniformSet,
        const AttributeSet& attributeSet) {
    addShaderFromSourceFile(QOpenGLShader::Vertex, resource(vertexShaderFile));
    addShaderFromSourceFile(QOpenGLShader::Fragment, resource(fragmentShaderFile));
    link();
    for (const auto& attribute : attributeSet.attributes()) {
        int location = QOpenGLShaderProgram::attributeLocation(attribute.m_name);
        m_attributeLocations.push_back(location);
    }
    for (const auto& uniform : uniformSet.uniforms()) {
        int location = QOpenGLShaderProgram::uniformLocation(uniform.m_name);
        m_uniformLocations.push_back(location);
    }
}

MaterialShader::~MaterialShader() = default;

QOpenGLShaderProgram& MaterialShader::glShader() {
    return *this;
}

int MaterialShader::attributeLocation(int attributeIndex) const {
    return m_attributeLocations[attributeIndex];
}

int MaterialShader::uniformLocation(int uniformIndex) const {
    return m_uniformLocations[uniformIndex];
}

Material* MaterialShader::lastModifiedByMaterial() const {
    return m_pLastModifiedByMaterial;
}
void MaterialShader::setLastModifiedByMaterial(Material* pMaterial) {
    m_pLastModifiedByMaterial = pMaterial;
}
