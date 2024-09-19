#include "rendergraph/materialshader.h"

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
