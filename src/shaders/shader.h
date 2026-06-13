#pragma once

#if defined(MIXXX_USE_QOPENGL)
#include <QOpenGLShaderProgram>
#elif QT_VERSION < 0x060000
#include <QGLShaderProgram>
#endif

#include "util/class.h"

namespace mixxx {
class Shader;
}

class mixxx::Shader
#ifdef MIXXX_USE_QOPENGL
        : public QOpenGLShaderProgram {
#else
        : public QGLShaderProgram {
#endif
  public:
    Shader();
    ~Shader();

  protected:
    void load(const QString& fragmentShader, const QString& vertexShader);

  private:
    DISALLOW_COPY_AND_ASSIGN(Shader)
};
