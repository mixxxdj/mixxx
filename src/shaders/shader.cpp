#include "shaders/shader.h"

#include "util/assert.h"

using namespace mixxx;

using GLShader = QOpenGLShader;

Shader::Shader() = default;

Shader::~Shader() = default;

void Shader::load(const QString& vertexShaderCode, const QString& fragmentShaderCode) {
    VERIFY_OR_DEBUG_ASSERT(addShaderFromSourceCode(
            GLShader::Vertex, vertexShaderCode)) {
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(addShaderFromSourceCode(
            GLShader::Fragment, fragmentShaderCode)) {
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(link()) {
        return;
    }
}
