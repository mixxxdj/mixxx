#include "glslwaveformrenderersignal.h"
#include "waveformwidgetrenderer.h"

#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"

#include "mathstuff.h"

#include <QGLFramebufferObject>

GLSLWaveformRendererSignal::GLSLWaveformRendererSignal(WaveformWidgetRenderer* waveformWidgetRenderer) :
    WaveformRendererSignalBase(waveformWidgetRenderer) {

    m_signalMaxShaderProgram = 0;
    m_frameShaderProgram = 0;

    m_textureId = 0;
    m_unitQuadListId = -1;

    m_loadedWaveform = 0;

    m_signalMaxbuffer = 0;
    m_framebuffer = 0;

    m_signalFrameBufferRatio = 2;
}

GLSLWaveformRendererSignal::~GLSLWaveformRendererSignal() {

    if( m_textureId)
        glDeleteTextures(1,&m_textureId);

    if( m_signalMaxShaderProgram) {
        m_signalMaxShaderProgram->removeAllShaders();
        delete m_signalMaxShaderProgram;
    }

    if( m_signalMaxbuffer)
        delete m_signalMaxbuffer;

    if( m_frameShaderProgram) {
        m_frameShaderProgram->removeAllShaders();
        delete m_frameShaderProgram;
    }

    if( m_framebuffer)
        delete m_framebuffer;
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

    return true;
}

bool GLSLWaveformRendererSignal::loadTexture() {
    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();
    Waveform* waveform = NULL;
    int dataSize = 0;
    WaveformData* data = NULL;

    if (trackInfo) {
        waveform = trackInfo->getWaveform();
        if (waveform != NULL) {
            dataSize = waveform->getDataSize();
            if (dataSize > 1) {
                data = waveform->data();
            }
        }
    }

    glEnable(GL_TEXTURE_2D);

    if (m_textureId == 0) {
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

    if (waveform != NULL && data != NULL) {
        int textureWidth = waveform->getTextureStride();
        int textureHeigth = waveform->getTextureSize() / waveform->getTextureStride();

        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,textureWidth,textureHeigth,0,GL_RGBA,GL_UNSIGNED_BYTE, data);
        int error = glGetError();
        if( error)
            qDebug() << "GLSLWaveformRendererSignal::loadTexture - glTexImage2D error" << error;
    } else {
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

    int bufferWidth = nearestSuperiorPowerOfTwo(m_waveformRenderer->getWidth()*3);
    int bufferHeight = nearestSuperiorPowerOfTwo(m_waveformRenderer->getHeight());

    m_signalMaxbuffer = new QGLFramebufferObject(bufferWidth/(m_signalFrameBufferRatio*2),2);

    if( m_framebuffer)
        delete m_framebuffer;

    //should work with any version of OpenGl
    m_framebuffer = new QGLFramebufferObject(bufferWidth,bufferHeight);

    if( !m_signalMaxbuffer || !m_framebuffer->isValid())
        qDebug() << "GLSLWaveformRendererSignal::createFrameBuffer - PBO not valid";

    //qDebug() << m_waveformRenderer->getWidth();
    //qDebug() << m_waveformRenderer->getWidth()*3;
    //qDebug() << bufferWidth;
}

void GLSLWaveformRendererSignal::onInit(){

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

void GLSLWaveformRendererSignal::onSetup(const QDomNode& /*node*/) {

}

void GLSLWaveformRendererSignal::onSetTrack(){
    m_loadedWaveform = 0;
    loadTexture();
}

void GLSLWaveformRendererSignal::onResize(){
    createFrameBuffer();
}

void GLSLWaveformRendererSignal::draw(QPainter* painter, QPaintEvent* /*event*/) {
    if (!m_framebuffer || !m_framebuffer->isValid()) {
        return;
    }

    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();
    if (!trackInfo) {
        return;
    }

    const Waveform* waveform = trackInfo->getWaveform();
    if (waveform == NULL) {
        return;
    }

    int dataSize = waveform->getDataSize();
    if (dataSize <= 1) {
        return;
    }

    const WaveformData* data = waveform->data();
    if (data == NULL) {
        return;
    }

    // save the GL state set for QPainter
    painter->beginNativePainting();

    //NOTE: (vRince) completion can change during loadTexture
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

    //compute signal max
    {
        glLoadIdentity();

        m_signalMaxShaderProgram->bind();
        glViewport(0,0,m_signalMaxbuffer->width(),m_signalMaxbuffer->height());

        // TODO(XXX) all these accesses of the waveform info need to be made
        // thread safe.
        m_signalMaxShaderProgram->setUniformValue("waveformLength", dataSize);
        m_signalMaxShaderProgram->setUniformValue("textureSize", waveform->getTextureSize());
        m_signalMaxShaderProgram->setUniformValue("textureStride", waveform->getTextureStride());
        m_signalMaxShaderProgram->setUniformValue("playPosition",(float)m_waveformRenderer->getPlayPos());
        m_signalMaxShaderProgram->setUniformValue("zoomFactor",(int)m_waveformRenderer->getZoomFactor());
        m_signalMaxShaderProgram->setUniformValue("width",m_signalMaxbuffer->width());
        m_signalMaxShaderProgram->setUniformValue("signalFrameBufferRatio",m_signalFrameBufferRatio);
        m_signalMaxShaderProgram->setUniformValue("gain",(float)m_waveformRenderer->getGain());

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

    //paint into frame buffer
    {
        glLoadIdentity();
        //glScalef( (float)m_signalMaxbuffer->width()/(float)m_framebuffer->width(), 1.0, 1.0);

        m_frameShaderProgram->bind();

        glViewport(0, 0, m_framebuffer->width(), m_framebuffer->height());

        m_frameShaderProgram->setUniformValue("lowColor",m_colors.getLowColor());
        m_frameShaderProgram->setUniformValue("midColor",m_colors.getMidColor());
        m_frameShaderProgram->setUniformValue("highColor",m_colors.getHighColor());

        glBindTexture(GL_TEXTURE_2D, m_signalMaxbuffer->texture());
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

        m_framebuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCallList(m_unitQuadListId);
        m_framebuffer->release();

        m_frameShaderProgram->release();
    }

    glLoadIdentity();
    float scale = (float)m_framebuffer->width()/(2.0*(float)m_waveformRenderer->getWidth());
    scale /= (1.0+m_waveformRenderer->getRateAdjust());

    //NOTE: (vrince) try to move the camera to limit the stepping effect of actual versus current position centering
    //The following code must be paired with the shader that compute signal value in texture/gemometry world
    /*const int visualSamplePerPixel = m_signalFrameBufferRatio * m_waveformRenderer->getZoomFactor();
    const int nearestCurrentIndex = int(floor(indexPosition));
    const float actualIndexPosition = indexPosition - float(nearestCurrentIndex%(2*visualSamplePerPixel));
    const float deltaPosition = (indexPosition - actualIndexPosition);
    const float range = float(visualSamplePerPixel * m_waveformRenderer->getWidth());
    const float deltaInGeometry = deltaPosition / range;*/

    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();
    double visualGain = factory->getVisualGain(WaveformWidgetFactory::All);
    visualGain *= m_waveformRenderer->getGain();

    glTranslatef( 0.0, 0.0, 0.0);
    glScalef(scale, visualGain, 1.0);

    /*
    //TODO: (vrince) make this line work sometime
    glBegin(GL_LINES); {
        glColor4f(m_axesColor.redF(),m_axesColor.greenF(),m_axesColor.blueF(),m_axesColor.alphaF());
        glVertex2f(0,0);
        glVertex2f(m_waveformRenderer->getWidth(),0);
    }
    glEnd();
    */

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
