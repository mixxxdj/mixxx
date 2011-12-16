#ifndef GLWAVEFORMRENDERERSIGNALSHADER_H
#define GLWAVEFORMRENDERERSIGNALSHADER_H

#include "waveformrendererabstract.h"

#include <Qt/QtOpenGL>

class QGLShaderProgram;
template<typename T> class ShaderVariable;

class GLWaveformRendererSignalShader : public WaveformRendererAbstract
{
public:
    explicit GLWaveformRendererSignalShader(WaveformWidgetRenderer* waveformWidgetRenderer);
    virtual ~GLWaveformRendererSignalShader();

    virtual void init();
    virtual void setup(const QDomNode& node);
    virtual void draw(QPainter* painter, QPaintEvent* event);

    bool loadShaders();

private:
    GLint m_quadListId;
    GLint m_textureId;

    //shaders
    QGLShaderProgram* m_shaderProgram;

    ShaderVariable<int>* m_waveformLength;
    ShaderVariable<int>* m_textureLength;
    ShaderVariable<int>* m_textureStride;
    ShaderVariable<int>* m_indexPosition;
    ShaderVariable<int>* m_displayRange;
};

#endif // GLWAVEFORMRENDERERSIGNALSHADER_H
