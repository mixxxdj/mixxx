#include "glwaveformrenderersignalshader.h"
#include "shadervariable.h"

GLWaveformRendererSignalShader::GLWaveformRendererSignalShader(WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract(waveformWidgetRenderer) {

    m_shaderProgram = 0;

    m_waveformLength = new ShaderVariable<int>("waveformLength");
    m_textureLength = new ShaderVariable<int>("textureLength");
    m_textureStride = new ShaderVariable<int>("textureStride");
    m_indexPosition = new ShaderVariable<int>("indexPosition");
    m_displayRange = new ShaderVariable<int>("displayRange");
}

GLWaveformRendererSignalShader::~GLWaveformRendererSignalShader() {
    if( m_shaderProgram)
    {
        m_shaderProgram->removeAllShaders();
        delete m_shaderProgram;
    }
    delete m_waveformLength;
    delete m_textureLength;
    delete m_textureStride;
    delete m_indexPosition;
    delete m_displayRange;
}

bool GLWaveformRendererSignalShader::loadShaders()
{
    if( !m_shaderProgram)
        return false;

    qDebug() << "GLWaveformRendererSignalShader::loadShaders";

    if( m_shaderProgram->isLinked())
        m_shaderProgram->release();

    m_shaderProgram->removeAllShaders();

    m_shaderProgram->addShaderFromSourceFile( QGLShader::Vertex, "./waveform/shaders/passthrough.vert");
    m_shaderProgram->addShaderFromSourceFile( QGLShader::Fragment, "./waveform/shaders/filteredsignal.frag");

    if( !m_shaderProgram->link())
    {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - " << m_shaderProgram->log();
        return false;
    }

    if( !m_shaderProgram->bind())
    {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - shadrers binding failed";
        return false;
    }

    //init locations
    m_waveformLength->initUniformLocation(m_shaderProgram);
    m_textureLength->initUniformLocation(m_shaderProgram);
    m_textureStride->initUniformLocation(m_shaderProgram);
    m_indexPosition->initUniformLocation(m_shaderProgram);
    m_displayRange->initUniformLocation(m_shaderProgram);

    //vRince here I do not check if all location are Ok since
    //shader optimization could not provide Id for unused variable
    return true;
}

void GLWaveformRendererSignalShader::init(){

    if( !m_shaderProgram)
        m_shaderProgram = new QGLShaderProgram();

    if( !loadShaders())
        return;

    //set default values
    m_waveformLength->setUniformValue(m_shaderProgram);
    m_textureLength->setUniformValue(m_shaderProgram);
    m_textureStride->setUniformValue(m_shaderProgram);
    m_indexPosition->setUniformValue(m_shaderProgram);
    m_displayRange->setUniformValue(m_shaderProgram);
}

void GLWaveformRendererSignalShader::setup(const QDomNode& node) {

}

void GLWaveformRendererSignalShader::draw(QPainter* painter, QPaintEvent* event) {

}
