#ifndef GLWAVEFORMRENDERERSIGNALSHADER_H
#define GLWAVEFORMRENDERERSIGNALSHADER_H

#include <QGLFramebufferObject>
#include <QGLShaderProgram>
#include <QtOpenGL>
#include <QOpenGLFunctions_2_1>

#include "track/track.h"
#include "util/memory.h"
#include "waveform/renderers/waveformrenderersignalbase.h"

class GLSLWaveformRendererSignal: public QObject,
        public WaveformRendererSignalBase,
        protected QOpenGLFunctions_2_1 {
    Q_OBJECT
  public:
    GLSLWaveformRendererSignal(WaveformWidgetRenderer* waveformWidgetRenderer,
                               bool rgbShader);
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
    bool m_rgbShader;
    std::unique_ptr<QGLShaderProgram> m_frameShaderProgram;
};

class GLSLWaveformRendererFilteredSignal: public GLSLWaveformRendererSignal {
public:
    GLSLWaveformRendererFilteredSignal(
            WaveformWidgetRenderer* waveformWidgetRenderer) :
            GLSLWaveformRendererSignal(waveformWidgetRenderer, false) {
    }
    ~GLSLWaveformRendererFilteredSignal() override {
    }
};

class GLSLWaveformRendererRGBSignal : public GLSLWaveformRendererSignal {
  public:
    GLSLWaveformRendererRGBSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : GLSLWaveformRendererSignal(waveformWidgetRenderer, true) {}
    ~GLSLWaveformRendererRGBSignal() override {}
};

#endif // GLWAVEFORMRENDERERSIGNALSHADER_H
