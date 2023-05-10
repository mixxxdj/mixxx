#pragma once

#include <QGLContext>
#include <QOpenGLFunctions_2_1>

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

class GLWaveformRenderer : protected QOpenGLFunctions_2_1 {
  public:
    GLWaveformRenderer()
            : m_pLastContext(nullptr) {
    }

    virtual void onInitializeGL() {
        initializeOpenGLFunctions();
    }

  protected:
    // Somehow QGLWidget does not call QGLWidget::initializeGL on macOS, so hack around that
    // by calling this in `draw` when the QGLContext has been made current.
    // TODO: remove this when upgrading to QOpenGLWidget
    void maybeInitializeGL() {
        if (QGLContext::currentContext() != m_pLastContext) {
            onInitializeGL();
            m_pLastContext = QGLContext::currentContext();
        }
    }

  private:
    const QGLContext* m_pLastContext;
};

#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
