#pragma once

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include <QGLFramebufferObject>
#include <QGLShaderProgram>
#include <QtOpenGL>

#include "track/track_decl.h"
#include "util/memory.h"
#include "waveform/renderers/waveformrenderersignalbase.h"

class GLSLWaveformRendererSignal : public QObject,
                                   public WaveformRendererSignalBase,
                                   public GLWaveformRenderer {
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
    void onInitializeGL() override;
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
    std::unique_ptr<QGLFramebufferObject> m_framebuffer;

    bool m_bDumpPng;

    // shaders
    bool m_shadersValid;
    ColorType m_colorType;
    const QString m_pFragShader;
    std::unique_ptr<QGLShaderProgram> m_frameShaderProgram;
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
