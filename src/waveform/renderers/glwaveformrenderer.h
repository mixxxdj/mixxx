#pragma once

#include <QtGui/qtgui-config.h> // for QT_NO_OPENGL and QT_OPENGL_ES_2

#if !defined(QT_NO_OPENGL)

#include <QOpenGLFunctions>

class GLWaveformRenderer : protected QOpenGLFunctions {
  public:
    virtual void initializeGL() {
        initializeOpenGLFunctions();
    }
};

#endif // !defined(QT_NO_OPENGL)
