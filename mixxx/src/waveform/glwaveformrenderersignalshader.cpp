#include "glwaveformrenderersignalshader.h"
#include "shadervariable.h"

#include "waveform/waveformwidgetrenderer.h"

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

void GLWaveformRendererSignalShader::createGeometry() {

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, 1.0, -1.0, -1000, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    m_quadListId = glGenLists(1);
    glNewList(m_quadListId, GL_COMPILE);
    {
        glBegin(GL_QUADS);
        {
            glTexCoord2f( -1.0,-1.0);
            glVertex3f(-1.0f,-1.0f, 0.0f);

            glTexCoord2f( 1.0,-1.0);
            glVertex3f( 1.0f,-1.0f, 0.0f);

            glTexCoord2f( 1.0, 1.0);
            glVertex3f( 1.0f, 1.0f, 0.0f);

            glTexCoord2f(-1.0, 1.0);
            glVertex3f(-1.0f, 1.0f, 0.0f);
        }
        glEnd();
    }
    glEndList();


    //Create a texture to hold the waveform data in GPU memory
    glEnable(GL_TEXTURE_2D);

    glGenTextures(1,&m_textureId);
    qDebug() << "dataLocation_" << m_textureId << "error" << glGetError();

    glBindTexture(GL_TEXTURE_2D, m_textureId);
    qDebug() << "bind error" << glGetError();

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //TODO
    //glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1024,1024,0,GL_RGBA,GL_UNSIGNED_BYTE,data_.constData());
    qDebug() << "glTexImage2D error" << glGetError();

    glDisable(GL_TEXTURE_2D);
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

    createGeometry();
}

void GLWaveformRendererSignalShader::setup(const QDomNode& node) {

}

void GLWaveformRendererSignalShader::draw(QPainter* painter, QPaintEvent* event) {

}
