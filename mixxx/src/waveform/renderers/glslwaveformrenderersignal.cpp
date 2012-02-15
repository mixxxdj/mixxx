#include "glslwaveformrenderersignal.h"
#include "waveformwidgetrenderer.h"

#include "waveform/shadervariable.h"
#include "waveform/waveform.h"

GLSLWaveformRendererSignal::GLSLWaveformRendererSignal(WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract(waveformWidgetRenderer) {

    m_shaderProgram = 0;
    m_textureId = 0;
    m_quadListId = -1;

    m_loadedWaveform = 0;

    m_waveformLength = new ShaderVariable<int>("waveformLength");
    m_textureSize = new ShaderVariable<int>("textureSize");
    m_textureStride = new ShaderVariable<int>("textureStride");
    m_indexPosition = new ShaderVariable<float>("indexPosition");
    m_displayRange = new ShaderVariable<float>("displayRange");
    m_viewportWidth = new ShaderVariable<int>("viewportWidth");
    m_viewportHeigth =  new ShaderVariable<int>("viewportHeight");
}

GLSLWaveformRendererSignal::~GLSLWaveformRendererSignal() {

    if( m_textureId)
        glDeleteTextures(1,&m_textureId);

    if( m_shaderProgram)
    {
        m_shaderProgram->removeAllShaders();
        delete m_shaderProgram;
    }

    delete m_waveformLength;
    delete m_textureSize;
    delete m_textureStride;
    delete m_indexPosition;
    delete m_displayRange;
    delete m_viewportWidth;
    delete m_viewportHeigth;
}

bool GLSLWaveformRendererSignal::loadShaders()
{
    if( !m_shaderProgram)
        return false;

    qDebug() << "GLWaveformRendererSignalShader::loadShaders";

    if( m_shaderProgram->isLinked())
        m_shaderProgram->release();

    m_shaderProgram->removeAllShaders();

    m_shaderProgram->addShaderFromSourceFile( QGLShader::Vertex, "./src/waveform/shaders/passthrough.vert");
    m_shaderProgram->addShaderFromSourceFile( QGLShader::Fragment, "./src/waveform/shaders/filteredsignal.frag");

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
    m_textureSize->initUniformLocation(m_shaderProgram);
    m_textureStride->initUniformLocation(m_shaderProgram);
    m_indexPosition->initUniformLocation(m_shaderProgram);
    m_displayRange->initUniformLocation(m_shaderProgram);

    m_viewportWidth->initUniformLocation(m_shaderProgram);
    m_viewportHeigth->initUniformLocation(m_shaderProgram);

    //vRince here I do not check if all location are Ok since
    //shader optimization could not provide Id for unused variable
    return true;
}

bool GLSLWaveformRendererSignal::loadTexture()
{
    qDebug() << "GLSLWaveformRendererSignal::loadTexture()";

    glEnable(GL_TEXTURE_2D);

    if( m_textureId == 0) {
        glGenTextures(1,&m_textureId);

        int error = glGetError();
        if( error)
            qDebug() << "GLSLWaveformRendererSignal::loadTexture - m_textureId" << m_textureId << "error" << error;
    }

    glBindTexture(GL_TEXTURE_2D, m_textureId);

    int error = glGetError();
    if( error)
        qDebug() << "GLSLWaveformRendererSignal::loadTexture - bind error" << error;

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    TrackPointer trackPoint = m_waveformRenderer->getTrackInfo();
    if( trackPoint) {
        Waveform* waveform = trackPoint->getWaveform();

        if( waveform) {
            int textureWidth = waveform->getTextureStride();
            int textureHeigth = waveform->getTextureSize() / waveform->getTextureStride();

            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,textureWidth,textureHeigth,0,GL_RGBA,GL_UNSIGNED_BYTE,waveform->data());

            int error = glGetError();
            if( error)
                qDebug() << "GLSLWaveformRendererSignal::loadTexture - glTexImage2D error" << error;
        }
    }
    else {
        glDeleteTextures(1,&m_textureId);
        m_textureId = 0;
    }

    glDisable(GL_TEXTURE_2D);

    return true;
}

void GLSLWaveformRendererSignal::createGeometry() {

    if( m_quadListId != -1)
        return;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1000, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    m_quadListId = glGenLists(1);
    glNewList(m_quadListId, GL_COMPILE);
    {
        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.0,0.0);
            glVertex3f(-1.0f,-1.0f,0.0f);

            glTexCoord2f(1.0, 0.0);
            glVertex3f(1.0f,-1.0f,0.0f);

            glTexCoord2f(1.0,1.0);
            glVertex3f(1.0f,1.0f, 0.0f);

            glTexCoord2f(0.0,1.0);
            glVertex3f(-1.0f,1.0f,0.0f);
        }
        glEnd();
    }
    glEndList();
}

void GLSLWaveformRendererSignal::init(){

    if( !m_shaderProgram)
        m_shaderProgram = new QGLShaderProgram();

    if( !loadShaders())
        return;

    m_loadedWaveform = 0;

    createGeometry();
    loadTexture();
}

void GLSLWaveformRendererSignal::setup(const QDomNode& node) {
    //TODO(vrince)
}

void GLSLWaveformRendererSignal::onSetTrack(){
    m_loadedWaveform = 0;
    loadTexture();
}

void GLSLWaveformRendererSignal::draw(QPainter* painter, QPaintEvent* event) {

    const TrackInfoObject* trackInfo = m_waveformRenderer->getTrackInfo().data();

    if( !trackInfo)
        return;

    const Waveform* waveform = trackInfo->getWaveform();

    // save the GL state set for QPainter
    painter->beginNativePainting();

    //NOTE(vRince) completion can change during laodTexture
    //do not remove currenCompletion
    const int currentCompletion = waveform->getCompletion();
    if( m_loadedWaveform < waveform->getCompletion())
    {
        loadTexture();
        m_loadedWaveform = currentCompletion;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, -1000, 1000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(.0f,.0f,.0f);
    glScalef(1.0,1.0,1.0);

    glViewport(0, 0, m_waveformRenderer->getWidth(), m_waveformRenderer->getHeight());

    m_shaderProgram->bind();

    const float currentPosition = m_waveformRenderer->getPlayPos()*waveform->size();
    const float range = (float)m_waveformRenderer->getWidth()*m_waveformRenderer->getVisualSamplePerPixel();

    m_waveformLength->setUniformValue(m_shaderProgram,waveform->size());
    m_textureSize->setUniformValue(m_shaderProgram,waveform->getTextureSize());
    m_textureStride->setUniformValue(m_shaderProgram,waveform->getTextureStride());
    m_displayRange->setUniformValue(m_shaderProgram,range);
    m_indexPosition->setUniformValue(m_shaderProgram,currentPosition);

    m_viewportWidth->setUniformValue(m_shaderProgram,m_waveformRenderer->getWidth());
    m_viewportHeigth->setUniformValue(m_shaderProgram,m_waveformRenderer->getHeight());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glCallList(m_quadListId);
    glDisable(GL_TEXTURE_2D);

    m_shaderProgram->release();

    glDisable(GL_ALPHA_TEST);

    glBegin(GL_LINE_LOOP);
    {
        glColor4f(0.1,0.1,0.1,0.5);
        glVertex3f(-1.0f,-1.0f, 0.0f);
        glVertex3f( 1.0f, 1.0f, 0.0f);
        glVertex3f( 1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 0.0f);
    }
    glEnd();

    glDisable(GL_BLEND);

    painter->endNativePainting();
}
