#pragma once

#include <QOpenGLShaderProgram>

#include "util/class.h"

namespace qopengl {
class Shader;
}

class qopengl::Shader : public QOpenGLShaderProgram {
  public:
    Shader();
    ~Shader();

  protected:
    void load(const QString& fragmentShader, const QString& vertexShader);

  private:
    DISALLOW_COPY_AND_ASSIGN(Shader)
};
