#ifndef GLWAVEFORMRENDERERSIGNALSHADER_H
#define GLWAVEFORMRENDERERSIGNALSHADER_H

#include <QGLFramebufferObject>
#include <QGLShaderProgram>
#include <QtOpenGL>

#include "waveformrenderersignalbase.h"

class GLSLWaveformRendererSignal : public QObject, public WaveformRendererSignalBase {
    Q_OBJECT
  public:
    explicit GLSLWaveformRendererSignal(
            WaveformWidgetRenderer* waveformWidgetRenderer, bool rgbShader);
    virtual ~GLSLWaveformRendererSignal();

    virtual bool onInit();
    virtual void onSetup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

    virtual void onSetTrack();
    virtual void onResize();

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

    int m_loadedWaveform;

    //Frame buffer for two pass rendering
    bool m_frameBuffersValid;
    QGLFramebufferObject* m_framebuffer;

    bool m_bDumpPng;

    // shaders
    bool m_shadersValid;
    bool m_rgbShader;
    QGLShaderProgram* m_frameShaderProgram;
};

class GLSLWaveformRendererFilteredSignal : public GLSLWaveformRendererSignal {
  public:
    GLSLWaveformRendererFilteredSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : GLSLWaveformRendererSignal(waveformWidgetRenderer, false) {}
    virtual ~GLSLWaveformRendererFilteredSignal() {}
};

class GLSLWaveformRendererRGBSignal : public GLSLWaveformRendererSignal {
  public:
    GLSLWaveformRendererRGBSignal(
        WaveformWidgetRenderer* waveformWidgetRenderer)
        : GLSLWaveformRendererSignal(waveformWidgetRenderer, true) {}
    virtual ~GLSLWaveformRendererRGBSignal() {}
};

#endif // GLWAVEFORMRENDERERSIGNALSHADER_H
