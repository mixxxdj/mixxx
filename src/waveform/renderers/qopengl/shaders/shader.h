#pragma once

#include <QOpenGLShaderProgram>

namespace qopengl {
class Shader;
}

class qopengl::Shader : public QOpenGLShaderProgram {
  public:
    Shader();
    ~Shader();

  protected:
    void load(const QString& fragmentShader, const QString& vertexShader);
};
