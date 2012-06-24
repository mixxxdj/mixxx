#ifndef GLWAVEFORMRENDERERSIGNALSHADER_H
#define GLWAVEFORMRENDERERSIGNALSHADER_H

#include "waveformrenderersignalbase.h"

#include <QGLFramebufferObject>
#include <QGLShaderProgram>
#include <QtOpenGL>

class GLSLWaveformRendererSignal : public WaveformRendererSignalBase {
public:
    explicit GLSLWaveformRendererSignal(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLSLWaveformRendererSignal();

    virtual void onInit();
    virtual void onSetup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

    virtual void onSetTrack();
    virtual void onResize();

    bool loadShaders();
    bool loadTexture();

private:
    void createGeometry();
    void createFrameBuffers();

    GLint m_unitQuadListId;
    GLuint m_textureId;

    int m_loadedWaveform;

    //Frame buffer for two pass rendering
    bool m_frameBuffersValid;
    QGLFramebufferObject* m_signalMaxbuffer;
    QGLFramebufferObject* m_framebuffer;

    int m_signalFrameBufferRatio;

    //shaders
    bool m_shardersValid;
    QGLShaderProgram* m_signalMaxShaderProgram;
    QGLShaderProgram* m_frameShaderProgram;
};

#endif // GLWAVEFORMRENDERERSIGNALSHADER_H
