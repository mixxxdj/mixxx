#pragma once

#include "waveform/renderers/deprecated/glwaveformrenderer.h"
#include "waveform/renderers/waveformrendererabstract.h"
#include "waveform/widgets/waveformwidgetabstract.h"
#include "widget/wglwidget.h"

QT_FORWARD_DECLARE_CLASS(QString)

/// GLWaveformWidgetAbstract is a WaveformWidgetAbstract & WGLWidget. Any added
/// renderers derived from GLWaveformRenderer can implement a virtual method
/// initializeGL, which will be called from GLWaveformRenderer::initializeGL
/// (which overrides WGLWidget::initializeGL). This can be used for initialization
/// that must be deferred until the GL context has been initialized and that can't
/// be done in the constructor.
class GLWaveformWidgetAbstract : public WaveformWidgetAbstract, public WGLWidget {
  public:
    GLWaveformWidgetAbstract(const QString& group, QWidget* parent);

    WGLWidget* getGLWidget() override {
        return this;
    }

  protected:
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
#ifdef MIXXX_USE_QOPENGL
    void paintGL() override {
        // Called by OpenGLWindow to avoid flickering on resize.
        // The static_cast of this in required to avoid ambiguity
        // as method render is in both base classes.
        static_cast<WaveformWidgetAbstract*>(this)->render();
    }
#endif
    void initializeGL() override {
        for (auto renderer : std::as_const(m_rendererStack)) {
            auto glRenderer = dynamic_cast<GLWaveformRenderer*>(renderer);
            if (glRenderer) {
                glRenderer->initializeGL();
            }
        }
    }

#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

  private:
    void wheelEvent(QWheelEvent* event) override;
};
