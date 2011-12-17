#ifndef GLWAVEFORMRENDERERSIGNALSHADER_H
#define GLWAVEFORMRENDERERSIGNALSHADER_H

#include "waveformrendererabstract.h"

#include <Qt/QtOpenGL>

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

    bool loadShaders();
    bool loadTexture();

private:
    void createGeometry();

private:
    GLint m_quadListId;
    GLuint m_textureId;

    //shaders
    QGLShaderProgram* m_shaderProgram;

    ShaderVariable<int>* m_waveformLength;
    ShaderVariable<int>* m_textureSize;
    ShaderVariable<int>* m_textureStride;
    ShaderVariable<int>* m_indexPosition;
    ShaderVariable<int>* m_displayRange;
};

#endif // GLWAVEFORMRENDERERSIGNALSHADER_H
