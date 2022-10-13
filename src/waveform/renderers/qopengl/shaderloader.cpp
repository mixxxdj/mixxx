#include "waveform/renderers/qopengl/shaderloader.h"

using namespace qopengl;

ShaderLoader::ShaderLoader() = default;

ShaderLoader::~ShaderLoader() = default;

void ShaderLoader::initShaders(const QString& vertexShaderCode, const QString& fragmentShaderCode) {
    if (!m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderCode)) {
        return;
    }

    if (!m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderCode)) {
        return;
    }

    if (!m_shaderProgram.link()) {
        return;
    }

    if (!m_shaderProgram.bind()) {
        return;
    }
}
