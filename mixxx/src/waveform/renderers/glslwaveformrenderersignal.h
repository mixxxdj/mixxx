#ifndef GLWAVEFORMRENDERERSIGNALSHADER_H
#define GLWAVEFORMRENDERERSIGNALSHADER_H

#include "waveformrendererabstract.h"
#include "waveformsignalcolors.h"

#include <QGLFramebufferObject>
#include <QGLShaderProgram>
#include <QtOpenGL>

template<typename T> class ShaderVariable;

class GLSLWaveformRendererSignal : public WaveformRendererAbstract {
public:
    explicit GLSLWaveformRendererSignal(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLSLWaveformRendererSignal();

    virtual void init();
    virtual void setup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

    virtual void onSetTrack();
    virtual void onResize();

    bool loadShaders();
    bool loadTexture();

private:
    void createGeometry();
    void createFrameBuffer();

    GLint m_unitQuadListId;

    GLuint m_textureId;

    int m_loadedWaveform;

    WaveformSignalColors m_colors;

    //Frame buffer for two pass rendering
    QGLFramebufferObject* m_signalMaxbuffer;
    QGLFramebufferObject* m_framebuffer;


    //shaders
    QGLShaderProgram* m_signalMaxShaderProgram;
    QGLShaderProgram* m_frameShaderProgram;

    ShaderVariable<int>* m_waveformLength;
    ShaderVariable<int>* m_textureSize;
    ShaderVariable<int>* m_textureStride;
    ShaderVariable<int>* m_zoomFactor;

    ShaderVariable<float>* m_indexPosition;

    ShaderVariable<int>* m_viewportWidth;
    ShaderVariable<int>* m_viewportHeigth;

    ShaderVariable<QColor>* m_lowColor;
    ShaderVariable<QColor>* m_midColor;
    ShaderVariable<QColor>* m_highColor;


};

#endif // GLWAVEFORMRENDERERSIGNALSHADER_H
