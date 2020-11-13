#pragma once

#include <QOpenGLFunctions_2_1>
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include <QGLFramebufferObject>
#include <QGLShaderProgram>
#include <QtOpenGL>

#include "track/track_decl.h"
#include "util/memory.h"
#include "waveform/renderers/waveformrenderersignalbase.h"

class GLSLWaveformRendererSignal: public QObject,
        public WaveformRendererSignalBase,
        protected QOpenGLFunctions_2_1 {
    Q_OBJECT
  public:
    enum class ColorType {
        Filtered,
        RGB,
        RGBFiltered,
    };

    GLSLWaveformRendererSignal(WaveformWidgetRenderer* waveformWidgetRenderer,
            ColorType colorType,
            const char* fragShader);
    ~GLSLWaveformRendererSignal() override;

    bool onInit() override;
    void onSetup(const QDomNode& node) override;
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
    const char* m_pFragShader;
    std::unique_ptr<QGLShaderProgram> m_frameShaderProgram;
};

class GLSLWaveformRendererFilteredSignal: public GLSLWaveformRendererSignal {
public:
  GLSLWaveformRendererFilteredSignal(
          WaveformWidgetRenderer* waveformWidgetRenderer)
          : GLSLWaveformRendererSignal(waveformWidgetRenderer,
                    ColorType::Filtered,
                    ":/shaders/filteredsignal.frag") {
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
                      ":/shaders/rgbsignal.frag") {
    }
    ~GLSLWaveformRendererRGBSignal() override {}
};

class GLSLWaveformRenderer3BandSignal : public GLSLWaveformRendererSignal {
  public:
    GLSLWaveformRenderer3BandSignal(
            WaveformWidgetRenderer* waveformWidgetRenderer)
            : GLSLWaveformRendererSignal(waveformWidgetRenderer,
                      ColorType::RGBFiltered,
                      ":/shaders/3bandsignal.frag") {
    }
    ~GLSLWaveformRenderer3BandSignal() override {
    }
};

#endif // QT_NO_OPENGL && !QT_OPENGL_ES_2
