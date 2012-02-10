#ifndef GLWAVEFORMRENDERERSIGNALSHADER_H
#define GLWAVEFORMRENDERERSIGNALSHADER_H

#include "waveformrendererabstract.h"

#include <QtOpenGL>

class QGLShaderProgram;
template<typename T> class ShaderVariable;

class GLSLWaveformRendererSignal : public WaveformRendererAbstract
{
public:
    explicit GLSLWaveformRendererSignal(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLSLWaveformRendererSignal();

    virtual void init();
    virtual void setup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

    virtual void onSetTrack();

    bool loadShaders();
    bool loadTexture();

private:
    void createGeometry();

private:
    GLint m_quadListId;
    GLuint m_textureId;

    int m_loadedWaveform;

    //shaders
    QGLShaderProgram* m_shaderProgram;

    ShaderVariable<int>* m_waveformLength;
    ShaderVariable<int>* m_textureSize;
    ShaderVariable<int>* m_textureStride;
    ShaderVariable<float>* m_indexPosition;
    ShaderVariable<float>* m_displayRange;

    ShaderVariable<int>* m_viewportWidth;
    ShaderVariable<int>* m_viewportHeigth;
};

#endif // GLWAVEFORMRENDERERSIGNALSHADER_H
