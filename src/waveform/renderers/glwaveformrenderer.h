#pragma once

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include <QOpenGLFunctions_2_1>

class GLWaveformRenderer : protected QOpenGLFunctions_2_1 {
  public:
    virtual void initializeGL() {
        initializeOpenGLFunctions();
    }
};

#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
