#include "shaders/shader.h"

#include "util/assert.h"

using namespace mixxx;

Shader::Shader() = default;

Shader::~Shader() = default;

void Shader::load(const QString& vertexShaderCode, const QString& fragmentShaderCode) {
    VERIFY_OR_DEBUG_ASSERT(addShaderFromSourceCode(
            QOpenGLShader::Vertex, vertexShaderCode)) {
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(addShaderFromSourceCode(
            QOpenGLShader::Fragment, fragmentShaderCode)) {
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(link()) {
        return;
    }
}
