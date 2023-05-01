#pragma once

#ifndef MIXXX_USE_QOPENGL
#include <QGLContext>
#endif
#include <QOpenGLFunctions_2_1>

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

class GLWaveformRenderer : protected QOpenGLFunctions_2_1 {
  public:
    GLWaveformRenderer()
#ifndef MIXXX_USE_QOPENGL
            ,
            m_pLastContext(nullptr)
#endif
    {
    }

    virtual void onInitializeGL() {
        initializeOpenGLFunctions();
    }

  protected:
    // Somehow QGLWidget does not call QGLWidget::initializeGL on macOS, so hack around that
    // by calling this in `draw` when the QGLContext has been made current.
    // TODO: remove this when upgrading to QOpenGLWidget
    void maybeInitializeGL() {
#ifndef MIXXX_USE_QOPENGL
        if (QGLContext::currentContext() != m_pLastContext) {
            onInitializeGL();
            m_pLastContext = QGLContext::currentContext();
        }
#endif
    }

  private:
#ifndef MIXXX_USE_QOPENGL
    const QGLContext* m_pLastContext;
#endif
};

#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
