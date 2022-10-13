#pragma once

#include "waveform/renderers/glwaveformrenderer.h"
#include "waveform/widgets/waveformwidgetabstract.h"
#include "widget/wglwidget.h"
#include "widget/wwaveformviewer.h"

QT_FORWARD_DECLARE_CLASS(QString)

/// GLWaveformWidgetAbstract is a WaveformWidgetAbstract & WGLWidget. Its optional
/// member GLWaveformRenderer* m_pGlRenderer can implement a virtual method
/// onInitializeGL, which will be called from GLWaveformRenderer::initializeGL
/// (which overrides WGLWidget::initializeGL). This can be used for initialization
/// that must be deferred until the GL context has been initialized and that can't
/// be done in the constructor.
class GLWaveformWidgetAbstract : public WaveformWidgetAbstract, public WGLWidget {
  public:
    GLWaveformWidgetAbstract(const QString& group, QWidget* parent)
            : WaveformWidgetAbstract(group),
              WGLWidget(parent)
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
              ,
              m_pGlRenderer(nullptr)
#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
    {
    }

    WGLWidget* getGLWidget() override {
        return this;
    }

  protected:
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
    void initializeGL() override {
        if (m_pGlRenderer) {
            m_pGlRenderer->onInitializeGL();
        }
    }

#ifdef MIXXX_USE_QOPENGL
    // We need to forward events coming from the QOpenGLWindow
    // (drag&drop, mouse) to the viewer
    void handleEventFromWindow(QEvent* ev) override {
        auto viewer = dynamic_cast<WWaveformViewer*>(parent());
        if (viewer) {
            viewer->handleEventFromWindow(ev);
        }
    }
#endif

    GLWaveformRenderer* m_pGlRenderer;
#endif // !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)
};
