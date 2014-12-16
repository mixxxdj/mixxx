#include <QGLFramebufferObject>

#include "waveform/renderers/glslwaveformrenderersignal.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"
#include "controlobjectthread.h"

GLSLWaveformRendererSignal::GLSLWaveformRendererSignal(WaveformWidgetRenderer* waveformWidgetRenderer)
        : WaveformRendererSignalBase(waveformWidgetRenderer) {
    m_shadersValid = false;
    m_frameShaderProgram = 0;

    m_textureId = 0;
    m_unitQuadListId = -1;

    m_loadedWaveform = 0;

    m_frameBuffersValid = false;
    m_framebuffer = 0;

}

GLSLWaveformRendererSignal::~GLSLWaveformRendererSignal() {

    if (m_textureId)
        glDeleteTextures(1,&m_textureId);

    if (m_frameShaderProgram) {
        m_frameShaderProgram->removeAllShaders();
        delete m_frameShaderProgram;
    }

    if (m_framebuffer)
        delete m_framebuffer;
}

void GLSLWaveformRendererSignal::debugClick() {
    loadShaders();
    m_bDumpPng = true;
}

bool GLSLWaveformRendererSignal::loadShaders() {
    qDebug() << "GLWaveformRendererSignalShader::loadShaders";
    m_shadersValid = false;

    if (m_frameShaderProgram->isLinked()) {
        m_frameShaderProgram->release();
    }

    m_frameShaderProgram->removeAllShaders();

    if (!m_frameShaderProgram->addShaderFromSourceFile(
            QGLShader::Vertex, ":shaders/passthrough.vert")) {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - "
                 << m_frameShaderProgram->log();
        return false;
    }
    if (!m_frameShaderProgram->addShaderFromSourceFile(
            QGLShader::Fragment, ":shaders/filteredsignal.frag")) {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - "
                 << m_frameShaderProgram->log();
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
    ConstWaveformPointer waveform;
    int dataSize = 0;
    const WaveformData* data = NULL;

    if (trackInfo) {
        waveform = trackInfo->getWaveform();
        if (waveform) {
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

    if (m_framebuffer)
        delete m_framebuffer;

    //should work with any version of OpenGl
    m_framebuffer = new QGLFramebufferObject(bufferWidth * 4, bufferHeight * 4);

    if (!m_framebuffer->isValid())
        qWarning() << "GLSLWaveformRendererSignal::createFrameBuffer - frame buffer not valid";

    m_frameBuffersValid = m_framebuffer->isValid();

    //qDebug() << m_waveformRenderer->getWidth();
    //qDebug() << m_waveformRenderer->getWidth()*3;
    //qDebug() << bufferWidth;
}

bool GLSLWaveformRendererSignal::onInit() {
    m_loadedWaveform = 0;

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

void GLSLWaveformRendererSignal::onSetTrack() {
    m_loadedWaveform = 0;
    loadTexture();
}

void GLSLWaveformRendererSignal::onResize() {
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

    ConstWaveformPointer waveform = trackInfo->getWaveform();
    if (waveform.isNull()) {
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
    getGains(&allGain, &lowGain, &midGain, &highGain);

    double firstVisualIndex = m_waveformRenderer->getFirstDisplayedPosition() * dataSize/2.0;
    double lastVisualIndex = m_waveformRenderer->getLastDisplayedPosition() * dataSize/2.0;

    // const int firstIndex = int(firstVisualIndex+0.5);
    // firstVisualIndex = firstIndex - firstIndex%2;

    // const int lastIndex = int(lastVisualIndex+0.5);
    // lastVisualIndex = lastIndex + lastIndex%2;

    //qDebug() << "GAIN" << allGain << lowGain << midGain << highGain;

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

        m_frameShaderProgram->bind();

        glViewport(0, 0, m_framebuffer->width(), m_framebuffer->height());

        m_frameShaderProgram->setUniformValue("framebufferSize", QVector2D(
            m_framebuffer->width(), m_framebuffer->height()));
        m_frameShaderProgram->setUniformValue("waveformLength", dataSize);
        m_frameShaderProgram->setUniformValue("textureSize", waveform->getTextureSize());
        m_frameShaderProgram->setUniformValue("textureStride", waveform->getTextureStride());

        m_frameShaderProgram->setUniformValue("firstVisualIndex", (float)firstVisualIndex);
        m_frameShaderProgram->setUniformValue("lastVisualIndex", (float)lastVisualIndex);

        m_frameShaderProgram->setUniformValue("allGain", allGain);
        m_frameShaderProgram->setUniformValue("lowGain", lowGain);
        m_frameShaderProgram->setUniformValue("midGain", midGain);
        m_frameShaderProgram->setUniformValue("highGain", highGain);

        m_frameShaderProgram->setUniformValue("axesColor", QVector4D(m_axesColor_r, m_axesColor_g,
                                                                     m_axesColor_b, m_axesColor_a));
        m_frameShaderProgram->setUniformValue("lowColor", QVector4D(m_lowColor_r, m_lowColor_g,
                                                                    m_lowColor_b, 1.0));
        m_frameShaderProgram->setUniformValue("midColor", QVector4D(m_midColor_r, m_midColor_g,
                                                                    m_midColor_b, 1.0));
        m_frameShaderProgram->setUniformValue("highColor", QVector4D(m_highColor_r, m_highColor_g,
                                                                    m_highColor_b, 1.0));

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        m_framebuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glCallList(m_unitQuadListId);

        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.0,0.0);
            glVertex3f(firstVisualIndex, -1.0f, 0.0f);

            glTexCoord2f(1.0, 0.0);
            glVertex3f(lastVisualIndex, -1.0f, 0.0f);

            glTexCoord2f(1.0,1.0);
            glVertex3f(lastVisualIndex, 1.0f, 0.0f);

            glTexCoord2f(0.0,1.0);
            glVertex3f(firstVisualIndex, 1.0f, 0.0f);
        }
        glEnd();


        m_framebuffer->release();

        m_frameShaderProgram->release();

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        if (m_bDumpPng) {
            m_framebuffer->toImage().save("m_framebuffer.png");
            m_bDumpPng = false;
        }
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -10.0, 10.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glTranslatef(0.0, 0.0, 0.0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    //paint buffer into viewport
    {
        glViewport(0, 0, m_waveformRenderer->getWidth(), m_waveformRenderer->getHeight());
        glBindTexture(GL_TEXTURE_2D, m_framebuffer->texture());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.0, 0.0);
            glVertex3f(-1.0f, -1.0f, 0.0f);

            glTexCoord2f(1.0, 0.0);
            glVertex3f(1.0f, -1.0f, 0.0f);

            glTexCoord2f(1.0, 1.0);
            glVertex3f(1.0f, 1.0f, 0.0f);

            glTexCoord2f(0.0, 1.0);
            glVertex3f(-1.0f, 1.0f, 0.0f);
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
