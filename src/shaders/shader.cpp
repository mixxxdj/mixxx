#include "shaders/shader.h"

#include "util/assert.h"

using namespace mixxx;

#ifdef MIXXX_USE_QOPENGL
using GLShader = QOpenGLShader;
#else
using GLShader = QGLShader;
#endif

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
