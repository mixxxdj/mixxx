#include "glslwaveformrenderersignal.h"
#include "waveformwidgetrenderer.h"

#include "waveform/shadervariable.h"
#include "waveform/waveform.h"

#include "mathstuff.h"

#include <QGLFramebufferObject>

GLSLWaveformRendererSignal::GLSLWaveformRendererSignal(WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererAbstract(waveformWidgetRenderer) {

    m_signalMaxShaderProgram = 0;
    m_frameShaderProgram = 0;

    m_textureId = 0;
    m_unitQuadListId = -1;

    m_loadedWaveform = 0;

    m_signalMaxbuffer = 0;
    m_framebuffer = 0;

    m_waveformLength = new ShaderVariable<int>("waveformLength");
    m_textureSize = new ShaderVariable<int>("textureSize");
    m_textureStride = new ShaderVariable<int>("textureStride");
    m_indexPosition = new ShaderVariable<float>("indexPosition");
    m_zoomFactor = new ShaderVariable<int>("zoomFactor");

    m_viewportWidth = new ShaderVariable<int>("viewportWidth");
    m_viewportHeigth =  new ShaderVariable<int>("viewportHeight");

    m_lowColor = new ShaderVariable<QColor>("lowColor");
    m_midColor = new ShaderVariable<QColor>("midColor");
    m_highColor = new ShaderVariable<QColor>("highColor");
}

GLSLWaveformRendererSignal::~GLSLWaveformRendererSignal() {

    if( m_textureId)
        glDeleteTextures(1,&m_textureId);

    if( m_signalMaxShaderProgram) {
        m_signalMaxShaderProgram->removeAllShaders();
        delete m_signalMaxShaderProgram;
    }

    if( m_frameShaderProgram) {
        m_frameShaderProgram->removeAllShaders();
        delete m_frameShaderProgram;
    }

    if( m_framebuffer)
        delete m_framebuffer;

    delete m_waveformLength;
    delete m_textureSize;
    delete m_textureStride;
    delete m_indexPosition;
    delete m_zoomFactor;
    delete m_viewportWidth;
    delete m_viewportHeigth;

    delete m_lowColor;
    delete m_midColor;
    delete m_highColor;
}

bool GLSLWaveformRendererSignal::loadShaders()
{
    qDebug() << "GLWaveformRendererSignalShader::loadShaders";

    if( m_signalMaxShaderProgram->isLinked())
        m_signalMaxShaderProgram->release();

    m_signalMaxShaderProgram->removeAllShaders();

    m_signalMaxShaderProgram->addShaderFromSourceFile( QGLShader::Vertex, "./src/waveform/shaders/passthrough.vert");
    m_signalMaxShaderProgram->addShaderFromSourceFile( QGLShader::Fragment, "./src/waveform/shaders/computemaxsignal.frag");

    if( !m_signalMaxShaderProgram->link())
    {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - " << m_signalMaxShaderProgram->log();
        return false;
    }

    if( !m_signalMaxShaderProgram->bind())
    {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - shadrers binding failed";
        return false;
    }

    //init locations
    /*
    m_textureSize->initUniformLocation(m_signalMaxShaderProgram);
    m_textureStride->initUniformLocation(m_signalMaxShaderProgram);
    m_indexPosition->initUniformLocation(m_signalMaxShaderProgram);
    m_zoomFactor->initUniformLocation(m_signalMaxShaderProgram);

    m_viewportWidth->initUniformLocation(m_signalMaxShaderProgram);
    m_viewportHeigth->initUniformLocation(m_signalMaxShaderProgram);
    */

    //****************

    if( m_frameShaderProgram->isLinked())
        m_frameShaderProgram->release();

    m_frameShaderProgram->removeAllShaders();

    m_frameShaderProgram->addShaderFromSourceFile( QGLShader::Vertex, "./src/waveform/shaders/passthrough.vert");
    m_frameShaderProgram->addShaderFromSourceFile( QGLShader::Fragment, "./src/waveform/shaders/filteredsignal.frag");

    if( !m_frameShaderProgram->link())
    {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - " << m_frameShaderProgram->log();
        return false;
    }

    if( !m_frameShaderProgram->bind())
    {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - shadrers binding failed";
        return false;
    }

    //init locations
    m_waveformLength->initUniformLocation(m_frameShaderProgram);
    m_textureSize->initUniformLocation(m_frameShaderProgram);
    m_textureStride->initUniformLocation(m_frameShaderProgram);
    m_indexPosition->initUniformLocation(m_frameShaderProgram);
    m_zoomFactor->initUniformLocation(m_frameShaderProgram);

    m_viewportWidth->initUniformLocation(m_frameShaderProgram);
    m_viewportHeigth->initUniformLocation(m_frameShaderProgram);

    m_lowColor->initUniformLocation(m_frameShaderProgram);
    m_midColor->initUniformLocation(m_frameShaderProgram);
    m_highColor->initUniformLocation(m_frameShaderProgram);


    //NOTE: vRince here I do not check if all location are Ok since
    //shader optimization could not provide Id for unused variable
    return true;
}

bool GLSLWaveformRendererSignal::loadTexture()
{
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

    if( m_unitQuadListId != -1)
        return;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -10.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    m_unitQuadListId = glGenLists(1);
    glNewList(m_unitQuadListId, GL_COMPILE);
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

void GLSLWaveformRendererSignal::createFrameBuffer()
{
    if( m_signalMaxbuffer)
        delete m_signalMaxbuffer;

    int bufferWidth = nearestSuperiorPowerOfTwo(m_waveformRenderer->getWidth()*2);

    m_signalMaxbuffer = new QGLFramebufferObject(bufferWidth/2,2);

    if( m_framebuffer)
        delete m_framebuffer;

    //should work with any version of OpenGl
    m_framebuffer = new QGLFramebufferObject(bufferWidth,
                                             nearestSuperiorPowerOfTwo(m_waveformRenderer->getHeight()));

    if( !m_signalMaxbuffer || !m_framebuffer->isValid())
        qDebug() << "GLSLWaveformRendererSignal::createFrameBuffer - PBO not valid";

}

void GLSLWaveformRendererSignal::init(){

    if(!m_signalMaxShaderProgram)
        m_signalMaxShaderProgram = new QGLShaderProgram();

    if(!m_frameShaderProgram)
        m_frameShaderProgram = new QGLShaderProgram();

    if( !loadShaders())
        return;

    m_loadedWaveform = 0;

    createGeometry();
    loadTexture();
}

void GLSLWaveformRendererSignal::setup(const QDomNode& node) {
    m_colors.setup(node);
    m_lowColor->setValue( m_colors.getLowColor());
    m_midColor->setValue( m_colors.getMidColor());
    m_highColor->setValue( m_colors.getHighColor());
}

void GLSLWaveformRendererSignal::onSetTrack(){
    m_loadedWaveform = 0;
    loadTexture();
}

void GLSLWaveformRendererSignal::onResize(){
    createFrameBuffer();
}

void GLSLWaveformRendererSignal::draw(QPainter* painter, QPaintEvent* /*event*/) {

    const TrackInfoObject* trackInfo = m_waveformRenderer->getTrackInfo().data();

    if( !trackInfo)
        return;

    if(!m_framebuffer || !m_framebuffer->isValid())
        return;

    const Waveform* waveform = trackInfo->getWaveform();

    // save the GL state set for QPainter
    painter->beginNativePainting();

    //NOTE(vRince) completion can change during loadTexture
    //do not remove currenCompletion temp variable !
    const int currentCompletion = waveform->getCompletion();
    if( m_loadedWaveform < currentCompletion)
    {
        loadTexture();
        m_loadedWaveform = currentCompletion;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -10.0, 10.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(.0f,.0f,.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    //compute max
    {
        glLoadIdentity();

        m_signalMaxShaderProgram->bind();
        glViewport(0,0,m_signalMaxbuffer->width(),m_signalMaxbuffer->height());
        const float indexPosition = m_waveformRenderer->getPlayPos()*(float)waveform->size();

        m_signalMaxShaderProgram->setUniformValue("waveformLength",waveform->size());
        m_signalMaxShaderProgram->setUniformValue("textureSize",waveform->getTextureSize());
        m_signalMaxShaderProgram->setUniformValue("textureStride",waveform->getTextureStride());
        m_signalMaxShaderProgram->setUniformValue("indexPosition",indexPosition);
        m_signalMaxShaderProgram->setUniformValue("zoomFactor",(int)m_waveformRenderer->getZoomFactor());
        m_signalMaxShaderProgram->setUniformValue("viewportWidth",m_signalMaxbuffer->width());

        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

        m_signalMaxbuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCallList(m_unitQuadListId);
        m_signalMaxbuffer->release();

        m_signalMaxShaderProgram->release();

        //m_signalMaxbuffer->toImage().save("m_signalMaxbuffer.png");
    }

    glLoadIdentity();

    //pbo/viewport scaling & pitch scaling
    //float scale = (float)m_framebuffer->width()/(float)m_waveformRenderer->getWidth();
    //scale /= (1.0+m_waveformRenderer->getRateAdjust());
    //glScalef(scale, 1.0, 1.0);

    //paint into frame buffer
    {
        glLoadIdentity();

        m_frameShaderProgram->bind();

        glViewport(0, 0, m_framebuffer->width(), m_framebuffer->height());

        /*
        const float currentPosition = m_waveformRenderer->getPlayPos()*(float)waveform->size();

        m_waveformLength->setUniformValue(waveform->size());
        m_textureSize->setUniformValue(waveform->getTextureSize());
        m_textureStride->setUniformValue(waveform->getTextureStride());

        m_zoomFactor->setUniformValue((int)m_waveformRenderer->getZoomFactor());

        m_indexPosition->setUniformValue(currentPosition);

        m_viewportWidth->setUniformValue(m_framebuffer->width());
        m_viewportHeigth->setUniformValue(m_framebuffer->height());

        //NOTE: (vrince) this should be only one on setup ?
        m_lowColor->setUniformValue();
        m_midColor->setUniformValue();
        m_highColor->setUniformValue();
        */

        glBindTexture(GL_TEXTURE_2D, m_signalMaxbuffer->texture());
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

        m_framebuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCallList(m_unitQuadListId);
        m_framebuffer->release();

        m_frameShaderProgram->release();
    }

    //debug
    //m_signalFramebuffer->toImage().save("signalPBO.png");

    glLoadIdentity();
    glScalef(2.0/(1.0+m_waveformRenderer->getRateAdjust()), 1.0, 1.0);

    //paint buffer into viewport
    {
        glViewport(0, 0, m_waveformRenderer->getWidth(), m_waveformRenderer->getHeight());
        glBindTexture(GL_TEXTURE_2D, m_framebuffer->texture());
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glCallList(m_unitQuadListId);
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);

    //DEBUG
    /*
    glBegin(GL_LINE_LOOP);
    {
        glColor4f(0.5,1.0,0.5,0.75);
        glVertex3f(-1.0f,-1.0f, 0.0f);
        glVertex3f( 1.0f, 1.0f, 0.0f);
        glVertex3f( 1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 0.0f);
    }
    glEnd();
    */

    glDisable(GL_BLEND);

    painter->endNativePainting();
}
