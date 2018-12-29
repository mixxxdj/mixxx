#ifndef GLWAVEFORMRENDERERSIGNALSHADER_H
#define GLWAVEFORMRENDERERSIGNALSHADER_H

#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QtOpenGL>

#include "track/track.h"
#include "util/memory.h"
#include "waveform/renderers/waveformrenderersignalbase.h"

class GLSLWaveformRendererSignal : public QObject, public WaveformRendererSignalBase {
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
    std::unique_ptr<QOpenGLFramebufferObject> m_framebuffer;

    bool m_bDumpPng;

    // shaders
    const bool m_rgbShader;
    std::unique_ptr<QOpenGLShaderProgram> m_frameShaderProgram;
};

class GLSLWaveformRendererFilteredSignal : public GLSLWaveformRendererSignal {
  public:
    GLSLWaveformRendererFilteredSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : GLSLWaveformRendererSignal(waveformWidgetRenderer, false) {}
    ~GLSLWaveformRendererFilteredSignal() override {}
};

class GLSLWaveformRendererRGBSignal : public GLSLWaveformRendererSignal {
  public:
    GLSLWaveformRendererRGBSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : GLSLWaveformRendererSignal(waveformWidgetRenderer, true) {}
    ~GLSLWaveformRendererRGBSignal() override {}
};

#endif // GLWAVEFORMRENDERERSIGNALSHADER_H
