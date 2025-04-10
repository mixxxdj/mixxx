#pragma once

#include "waveform/renderers/deprecated/glwaveformrenderersignal.h"
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include <memory>

#include "track/track_decl.h"

#ifdef MIXXX_USE_QOPENGL
class QOpenGLFramebufferObject;
class QOpenGLShader;
class QOpenGLShaderProgram;
#else
class QGLFramebufferObject;
class QGLShader;
class QGLShaderProgram;
#endif

class GLSLWaveformRendererSignal : public QObject,
                                   public GLWaveformRendererSignal {
#ifdef MIXXX_USE_QOPENGL
    using FrameBufferObject = QOpenGLFramebufferObject;
    using Shader = QOpenGLShader;
    using ShaderProgram = QOpenGLShaderProgram;
#else
    using FrameBufferObject = QGLFramebufferObject;
    using Shader = QGLShader;
    using ShaderProgram = QGLShaderProgram;
#endif
    Q_OBJECT
  public:
    enum class ColorType {
        Filtered,
        RGB,
        RGBFiltered,
    };

    GLSLWaveformRendererSignal(WaveformWidgetRenderer* waveformWidgetRenderer,
            ColorType colorType,
            const QString& fragShader);
    ~GLSLWaveformRendererSignal() override;

    void onSetup(const QDomNode& node) override;
    void initializeGL() override;
    void draw(QPainter* painter, QPaintEvent* event) override;

    void onSetTrack() override;
    void onResize() override;

    void debugClick();
    bool loadShaders();
    bool loadTexture();

  public slots:
    void slotWaveformUpdated();

  private:
    void createGeometry();
    void createFrameBuffers();

    GLint m_unitQuadListId;
    GLuint m_textureId;

    TrackPointer m_loadedTrack;
    int m_textureRenderedWaveformCompletion;

    // Frame buffer for two pass rendering.
    std::unique_ptr<FrameBufferObject> m_framebuffer;

    bool m_bDumpPng;

    // shaders
    bool m_shadersValid;
    ColorType m_colorType;
    const QString m_pFragShader;
    std::unique_ptr<ShaderProgram> m_frameShaderProgram;
};

class GLSLWaveformRendererFilteredSignal: public GLSLWaveformRendererSignal {
public:
  GLSLWaveformRendererFilteredSignal(
          WaveformWidgetRenderer* waveformWidgetRenderer)
          : GLSLWaveformRendererSignal(waveformWidgetRenderer,
                    ColorType::Filtered,
                    QLatin1String(":/shaders/filteredsignal.frag")) {
  }
    ~GLSLWaveformRendererFilteredSignal() override {
    }
};

class GLSLWaveformRendererRGBSignal : public GLSLWaveformRendererSignal {
  public:
    GLSLWaveformRendererRGBSignal(
            WaveformWidgetRenderer* waveformWidgetRenderer)
            : GLSLWaveformRendererSignal(waveformWidgetRenderer,
                      ColorType::RGB,
                      QLatin1String(":/shaders/rgbsignal.frag")) {
    }
    ~GLSLWaveformRendererRGBSignal() override {}
};

class GLSLWaveformRendererStackedSignal : public GLSLWaveformRendererSignal {
  public:
    GLSLWaveformRendererStackedSignal(
            WaveformWidgetRenderer* waveformWidgetRenderer)
            : GLSLWaveformRendererSignal(waveformWidgetRenderer,
                      ColorType::RGBFiltered,
                      QLatin1String(":/shaders/stackedsignal.frag")) {
    }
    ~GLSLWaveformRendererStackedSignal() override {
    }
};

#endif // QT_NO_OPENGL && !QT_OPENGL_ES_2
