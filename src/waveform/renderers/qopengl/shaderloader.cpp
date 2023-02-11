#include "waveform/renderers/qopengl/shaderloader.h"

#include "util/assert.h"

using namespace qopengl;

ShaderLoader::ShaderLoader() = default;

ShaderLoader::~ShaderLoader() = default;

void ShaderLoader::initShaders(const QString& vertexShaderCode, const QString& fragmentShaderCode) {
    VERIFY_OR_DEBUG_ASSERT(m_shaderProgram.addShaderFromSourceCode(
            QOpenGLShader::Vertex, vertexShaderCode)) {
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(m_shaderProgram.addShaderFromSourceCode(
            QOpenGLShader::Fragment, fragmentShaderCode)) {
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(m_shaderProgram.link()) {
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(m_shaderProgram.bind()) {
        return;
    }
}
