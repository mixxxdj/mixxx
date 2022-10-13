#pragma once

#include <QOpenGLShaderProgram>

namespace qopengl {
class ShaderLoader;
}

class qopengl::ShaderLoader {
  public:
    ShaderLoader();
    ~ShaderLoader();

  protected:
    QOpenGLShaderProgram m_shaderProgram;

    void initShaders(const QString& fragmentShader, const QString& vertexShader);
};
