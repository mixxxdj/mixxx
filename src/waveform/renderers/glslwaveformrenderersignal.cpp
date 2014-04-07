#include <QGLFramebufferObject>

#include "waveform/renderers/glslwaveformrenderersignal.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"
#include "controlobjectthread.h"

#include "mathstuff.h"

GLSLWaveformRendererSignal::GLSLWaveformRendererSignal(WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererSignalBase(waveformWidgetRenderer) {
    m_shadersValid = false;
    m_signalMaxShaderProgram = 0;
    m_frameShaderProgram = 0;

    m_textureId = 0;
    m_unitQuadListId = -1;

    m_loadedWaveform = 0;

    m_frameBuffersValid = false;
    m_signalMaxbuffer = 0;
    m_framebuffer = 0;

    m_signalFrameBufferRatio = 2;
}

GLSLWaveformRendererSignal::~GLSLWaveformRendererSignal() {

    if (m_textureId)
        glDeleteTextures(1,&m_textureId);

    if (m_signalMaxShaderProgram) {
        m_signalMaxShaderProgram->removeAllShaders();
        delete m_signalMaxShaderProgram;
    }

    if (m_signalMaxbuffer)
        delete m_signalMaxbuffer;

    if (m_frameShaderProgram) {
        m_frameShaderProgram->removeAllShaders();
        delete m_frameShaderProgram;
    }

    if (m_framebuffer)
        delete m_framebuffer;
}

bool GLSLWaveformRendererSignal::loadShaders() {
    qDebug() << "GLWaveformRendererSignalShader::loadShaders";
    m_shadersValid = false;

    if (m_signalMaxShaderProgram->isLinked()) {
        m_signalMaxShaderProgram->release();
    }

    m_signalMaxShaderProgram->removeAllShaders();

    if (!m_signalMaxShaderProgram->addShaderFromSourceFile(
            QGLShader::Vertex, "./res/shaders/passthrough.vert")) {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - "
                 << m_signalMaxShaderProgram->log();
        return false;
    }
    if (!m_signalMaxShaderProgram->addShaderFromSourceFile(
            QGLShader::Fragment, "./res/shaders/computemaxsignal.frag")) {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - "
                 << m_signalMaxShaderProgram->log();
        return false;
    }
    if (!m_signalMaxShaderProgram->link()) {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - " << m_signalMaxShaderProgram->log();
        return false;
    }

    if (!m_signalMaxShaderProgram->bind()) {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - shadrers binding failed";
        return false;
    }

    if (m_frameShaderProgram->isLinked()) {
        m_frameShaderProgram->release();
    }

    m_frameShaderProgram->removeAllShaders();

    if (!m_frameShaderProgram->addShaderFromSourceFile(
            QGLShader::Vertex, "./res/shaders/passthrough.vert")) {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - "
                 << m_signalMaxShaderProgram->log();
        return false;
    }
    if (!m_frameShaderProgram->addShaderFromSourceFile(
            QGLShader::Fragment, "./res/shaders/filteredsignal.frag")) {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - "
                 << m_signalMaxShaderProgram->log();
        return false;
    }
    if (!m_frameShaderProgram->link()) {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - " << m_frameShaderProgram->log();
        return false;
    }

    if (!m_frameShaderProgram->bind()) {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - shaders binding failed";
        return false;
    }

    m_shadersValid = true;
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
        glGenTextures(1, &m_textureId);

        int error = glGetError();
        if (error)
            qDebug() << "GLSLWaveformRendererSignal::loadTexture - m_textureId" << m_textureId << "error" << error;
    }

    glBindTexture(GL_TEXTURE_2D, m_textureId);

    int error = glGetError();
    if (error)
        qDebug() << "GLSLWaveformRendererSignal::loadTexture - bind error" << error;

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    if (waveform != NULL && data != NULL) {
        // Waveform ensures that getTextureSize is a multiple of
        // getTextureStride so there is no rounding here.
        int textureWidth = waveform->getTextureStride();
        int textureHeigth = waveform->getTextureSize() / waveform->getTextureStride();

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeigth, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, data);
        int error = glGetError();
        if (error)
            qDebug() << "GLSLWaveformRendererSignal::loadTexture - glTexImage2D error" << error;
    } else {
        glDeleteTextures(1,&m_textureId);
        m_textureId = 0;
    }

    glDisable(GL_TEXTURE_2D);

    return true;
}

void GLSLWaveformRendererSignal::createGeometry() {

    if (m_unitQuadListId != -1)
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

void GLSLWaveformRendererSignal::createFrameBuffers()
{
    m_frameBuffersValid = false;

    int bufferWidth = m_waveformRenderer->getWidth();
    int bufferHeight = m_waveformRenderer->getHeight();

    if (m_signalMaxbuffer)
        delete m_signalMaxbuffer;

    m_signalMaxbuffer = new QGLFramebufferObject(bufferWidth, 2);

    if (!m_signalMaxbuffer->isValid())
        qWarning() << "GLSLWaveformRendererSignal::createFrameBuffer - signal frame buffer not valid";

    if (m_framebuffer)
        delete m_framebuffer;

    //should work with any version of OpenGl
    m_framebuffer = new QGLFramebufferObject(bufferWidth, bufferHeight);

    if (!m_framebuffer->isValid())
        qWarning() << "GLSLWaveformRendererSignal::createFrameBuffer - frame buffer not valid";

    m_frameBuffersValid = m_framebuffer->isValid() && m_signalMaxbuffer->isValid();

    //qDebug() << m_waveformRenderer->getWidth();
    //qDebug() << m_waveformRenderer->getWidth()*3;
    //qDebug() << bufferWidth;
}

bool GLSLWaveformRendererSignal::onInit(){
    m_loadedWaveform = 0;

    if (!m_signalMaxShaderProgram)
        m_signalMaxShaderProgram = new QGLShaderProgram();

    if (!m_frameShaderProgram)
        m_frameShaderProgram = new QGLShaderProgram();

    if (!loadShaders()) {
        return false;
    }
    createGeometry();
    if (!loadTexture()) {
        return false;
    }

    return true;
}

void GLSLWaveformRendererSignal::onSetup(const QDomNode& /*node*/) {

}

void GLSLWaveformRendererSignal::onSetTrack(){
    m_loadedWaveform = 0;
    loadTexture();
}

void GLSLWaveformRendererSignal::onResize(){
    createFrameBuffers();
}

void GLSLWaveformRendererSignal::draw(QPainter* painter, QPaintEvent* /*event*/) {
    if (!m_frameBuffersValid || !m_shadersValid) {
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
    if (m_loadedWaveform < currentCompletion)
    {
        loadTexture();
        m_loadedWaveform = currentCompletion;
    }

    // Per-band gain from the EQ knobs.
    float lowGain(1.0), midGain(1.0), highGain(1.0), allGain(1.0);
    if (m_pLowFilterControlObject &&
            m_pMidFilterControlObject &&
            m_pHighFilterControlObject) {
        lowGain = m_pLowFilterControlObject->get();
        midGain = m_pMidFilterControlObject->get();
        highGain = m_pHighFilterControlObject->get();
    }
    allGain = m_waveformRenderer->getGain();

    WaveformWidgetFactory* factory = WaveformWidgetFactory::instance();
    allGain *= factory->getVisualGain(::WaveformWidgetFactory::All);
    lowGain *= factory->getVisualGain(WaveformWidgetFactory::Low);
    midGain *= factory->getVisualGain(WaveformWidgetFactory::Mid);
    highGain *= factory->getVisualGain(WaveformWidgetFactory::High);


    double firstVisualIndex = m_waveformRenderer->getFirstDisplayedPosition() * dataSize/2.0;
    double lastVisualIndex = m_waveformRenderer->getLastDisplayedPosition() * dataSize/2.0;

    // const int firstIndex = int(firstVisualIndex+0.5);
    // firstVisualIndex = firstIndex - firstIndex%2;

    // const int lastIndex = int(lastVisualIndex+0.5);
    // lastVisualIndex = lastIndex + lastIndex%2;

    //qDebug() << "GAIN" << allGain << lowGain << midGain << highGain;

    //compute signal max
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(firstVisualIndex, lastVisualIndex, -1.0, 1.0, -10.0, 10.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(.0f,.0f,.0f);

        m_signalMaxShaderProgram->bind();
        glViewport(0, 0, m_signalMaxbuffer->width(), m_signalMaxbuffer->height());

        // TODO(XXX) all these accesses of the waveform info need to be made
        // thread safe.
        m_signalMaxShaderProgram->setUniformValue("waveformLength", dataSize);
        m_signalMaxShaderProgram->setUniformValue("textureSize", waveform->getTextureSize());
        m_signalMaxShaderProgram->setUniformValue("textureStride", waveform->getTextureStride());
        m_signalMaxShaderProgram->setUniformValue("playPosition",(float)m_waveformRenderer->getPlayPos());
        m_signalMaxShaderProgram->setUniformValue("zoomFactor",(int)m_waveformRenderer->getZoomFactor());
        m_signalMaxShaderProgram->setUniformValue("width",m_signalMaxbuffer->width());
        m_signalMaxShaderProgram->setUniformValue("signalFrameBufferRatio",m_signalFrameBufferRatio);

        m_signalMaxShaderProgram->setUniformValue("firstVisualIndex", (float)firstVisualIndex);
        m_signalMaxShaderProgram->setUniformValue("lastVisualIndex", (float)lastVisualIndex);

        m_signalMaxShaderProgram->setUniformValue("allGain", allGain);
        m_signalMaxShaderProgram->setUniformValue("lowGain", lowGain);
        m_signalMaxShaderProgram->setUniformValue("midGain", midGain);
        m_signalMaxShaderProgram->setUniformValue("highGain", highGain);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        m_signalMaxbuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glCallList(m_unitQuadListId);

        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.0,0.0);
            glVertex3f(firstVisualIndex,-1.0f,0.0f);

            glTexCoord2f(1.0, 0.0);
            glVertex3f(lastVisualIndex,-1.0f,0.0f);

            glTexCoord2f(1.0,1.0);
            glVertex3f(lastVisualIndex,1.0f, 0.0f);

            glTexCoord2f(0.0,1.0);
            glVertex3f(firstVisualIndex,1.0f,0.0f);
        }
        glEnd();


        m_signalMaxbuffer->release();

        m_signalMaxShaderProgram->release();

        //m_signalMaxbuffer->toImage().save("m_signalMaxbuffer.png");
    }

    glLoadIdentity();

    //paint into frame buffer
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(firstVisualIndex, lastVisualIndex, -1.0, 1.0, -10.0, 10.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(.0f,.0f,.0f);

        //glScalef((float)m_signalMaxbuffer->width()/(float)m_framebuffer->width(), 1.0, 1.0);

        m_frameShaderProgram->bind();

        glViewport(0, 0, m_framebuffer->width(), m_framebuffer->height());

        m_frameShaderProgram->setUniformValue("lowColor", m_pColors->getLowColor());
        m_frameShaderProgram->setUniformValue("midColor", m_pColors->getMidColor());
        m_frameShaderProgram->setUniformValue("highColor", m_pColors->getHighColor());

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_signalMaxbuffer->texture());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        m_framebuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glCallList(m_unitQuadListId);

        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.0,0.0);
            glVertex3f(firstVisualIndex,-1.0f,0.0f);

            glTexCoord2f(1.0, 0.0);
            glVertex3f(lastVisualIndex,-1.0f,0.0f);

            glTexCoord2f(1.0,1.0);
            glVertex3f(lastVisualIndex,1.0f, 0.0f);

            glTexCoord2f(0.0,1.0);
            glVertex3f(firstVisualIndex,1.0f,0.0f);
        }
        glEnd();


        m_framebuffer->release();

        m_frameShaderProgram->release();
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(firstVisualIndex, lastVisualIndex, -1.0, 1.0, -10.0, 10.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    //float scale = (float)m_framebuffer->width()/(2.0*(float)m_waveformRenderer->getWidth());
    //scale /= (1.0+m_waveformRenderer->getRateAdjust());

    //NOTE: (vrince) try to move the camera to limit the stepping effect of actual versus current position centering
    //The following code must be paired with the shader that compute signal value in texture/gemometry world
    /*const int visualSamplePerPixel = m_signalFrameBufferRatio * m_waveformRenderer->getZoomFactor();
    const int nearestCurrentIndex = int(floor(indexPosition));
    const float actualIndexPosition = indexPosition - float(nearestCurrentIndex%(2*visualSamplePerPixel));
    const float deltaPosition = (indexPosition - actualIndexPosition);
    const float range = float(visualSamplePerPixel * m_waveformRenderer->getWidth());
    const float deltaInGeometry = deltaPosition / range;*/

    glTranslatef(0.0, 0.0, 0.0);
    //glScalef(scale, 1.0, 1.0);

    /*
    //TODO: (vrince) make this line work sometime
    glBegin(GL_LINES); {
        glColor4f(m_axesColor_r, m_axesColor_g, m_axesColor_b, m_axesColor_a);
        glVertex2f(0,0);
        glVertex2f(m_waveformRenderer->getWidth(),0);
    }
    glEnd();
    */

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    //paint buffer into viewport
    {
        glViewport(0, 0, m_waveformRenderer->getWidth(), m_waveformRenderer->getHeight());
        glBindTexture(GL_TEXTURE_2D, m_framebuffer->texture());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.0,0.0);
            glVertex3f(firstVisualIndex,-1.0f,0.0f);

            glTexCoord2f(1.0, 0.0);
            glVertex3f(lastVisualIndex,-1.0f,0.0f);

            glTexCoord2f(1.0,1.0);
            glVertex3f(lastVisualIndex,1.0f, 0.0f);

            glTexCoord2f(0.0,1.0);
            glVertex3f(firstVisualIndex,1.0f,0.0f);
        }
        glEnd();

    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);

    //DEBUG
    /*
    glBegin(GL_LINE_LOOP);
    {
        glColor4f(0.5,1.0,0.5,0.75);
        glVertex3f(-1.0f,-1.0f, 0.0f);
        glVertex3f(1.0f, 1.0f, 0.0f);
        glVertex3f(1.0f,-1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 0.0f);
    }
    glEnd();
    */

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    painter->endNativePainting();
}
