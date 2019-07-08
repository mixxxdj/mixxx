#include <QGLFramebufferObject>

#include "waveform/renderers/glslwaveformrenderersignal.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"

GLSLWaveformRendererSignal::GLSLWaveformRendererSignal(WaveformWidgetRenderer* waveformWidgetRenderer,
                                                       bool rgbShader)
        : WaveformRendererSignalBase(waveformWidgetRenderer),
          m_unitQuadListId(-1),
          m_textureId(0),
          m_textureRenderedWaveformCompletion(0),
          m_bDumpPng(false),
          m_shadersValid(false),
          m_rgbShader(rgbShader) {
    initializeOpenGLFunctions();
}

GLSLWaveformRendererSignal::~GLSLWaveformRendererSignal() {
    if (m_textureId) {
        glDeleteTextures(1,&m_textureId);
    }

    if (m_frameShaderProgram) {
        m_frameShaderProgram->removeAllShaders();
    }
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
    QString fragmentShader = m_rgbShader ?
            ":/shaders/rgbsignal.frag" :
            ":/shaders/filteredsignal.frag";
    if (!m_frameShaderProgram->addShaderFromSourceFile(
            QGLShader::Fragment, fragmentShader)) {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - "
                 << m_frameShaderProgram->log();
        return false;
    }

    if (!m_frameShaderProgram->link()) {
        qDebug() << "GLWaveformRendererSignalShader::loadShaders - "
                 << m_frameShaderProgram->log();
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
    const WaveformData* data = nullptr;

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
        if (error) {
            qDebug() << "GLSLWaveformRendererSignal::loadTexture - m_textureId" << m_textureId << "error" << error;
        }
    }

    glBindTexture(GL_TEXTURE_2D, m_textureId);

    int error = glGetError();
    if (error) {
        qDebug() << "GLSLWaveformRendererSignal::loadTexture - bind error" << error;
    }

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    if (waveform != nullptr && data != nullptr) {
        // Waveform ensures that getTextureSize is a multiple of
        // getTextureStride so there is no rounding here.
        int textureWidth = waveform->getTextureStride();
        int textureHeight = waveform->getTextureSize() / waveform->getTextureStride();

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, data);
        int error = glGetError();
        if (error) {
            qDebug() << "GLSLWaveformRendererSignal::loadTexture - glTexImage2D error" << error;
        }
    } else {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
    }

    glDisable(GL_TEXTURE_2D);

    return true;
}

void GLSLWaveformRendererSignal::createGeometry() {

    if (m_unitQuadListId != -1) {
        return;
    }

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

void GLSLWaveformRendererSignal::createFrameBuffers() {
    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    // We create a frame buffer that is 4x the size of the renderer itself to
    // "oversample" the texture relative to the surface we're drawing on.
    const int oversamplingFactor = 4;
    const int bufferWidth = oversamplingFactor * m_waveformRenderer->getWidth() * devicePixelRatio;
    const int bufferHeight = oversamplingFactor * m_waveformRenderer->getHeight() * devicePixelRatio;

    m_framebuffer = std::make_unique<QGLFramebufferObject>(bufferWidth,
                                                           bufferHeight);

    if (!m_framebuffer->isValid()) {
        qWarning() << "GLSLWaveformRendererSignal::createFrameBuffer - frame buffer not valid";
    }
}

bool GLSLWaveformRendererSignal::onInit() {
    m_textureRenderedWaveformCompletion = 0;

    if (!m_frameShaderProgram) {
        m_frameShaderProgram = std::make_unique<QGLShaderProgram>();
    }

    if (!loadShaders()) {
        return false;
    }
    createGeometry();
    if (!loadTexture()) {
        return false;
    }

    return true;
}

void GLSLWaveformRendererSignal::onSetup(const QDomNode& node) {
    Q_UNUSED(node);
}

void GLSLWaveformRendererSignal::onSetTrack() {
    if (m_loadedTrack) {
        disconnect(m_loadedTrack.get(),
                &Track::waveformUpdated,
                this,
                &GLSLWaveformRendererSignal::slotWaveformUpdated);
    }

    slotWaveformUpdated();

    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        return;
    }

    // When the track's waveform has been changed (or cleared), it is necessary
    // to update (or delete) the texture containing the waveform which was
    // uploaded to GPU. Otherwise, previous waveform will be shown.
    connect(pTrack.get(),
            &Track::waveformUpdated,
            this,
            &GLSLWaveformRendererSignal::slotWaveformUpdated);

    m_loadedTrack = pTrack;
}

void GLSLWaveformRendererSignal::onResize() {
    createFrameBuffers();
}

void GLSLWaveformRendererSignal::slotWaveformUpdated() {
    m_textureRenderedWaveformCompletion = 0;
    loadTexture();
}

void GLSLWaveformRendererSignal::draw(QPainter* painter, QPaintEvent* /*event*/) {
    if (!m_framebuffer || !m_framebuffer->isValid() || !m_shadersValid) {
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
    if (data == nullptr) {
        return;
    }

    // save the GL state set for QPainter
    painter->beginNativePainting();

    // NOTE(vRince): completion can change during loadTexture
    // do not remove currenCompletion temp variable !
    const int currentCompletion = waveform->getCompletion();
    if (m_textureRenderedWaveformCompletion < currentCompletion) {
        loadTexture();
        m_textureRenderedWaveformCompletion = currentCompletion;
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
        if (m_orientation == Qt::Vertical) {
            glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
            glScalef(-1.0f, 1.0f, 1.0f);
        }
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

        QVector4D lowColor = m_rgbShader ?
                QVector4D(m_rgbLowColor_r, m_rgbLowColor_g, m_rgbLowColor_b, 1.0) :
                QVector4D(m_lowColor_r, m_lowColor_g, m_lowColor_b, 1.0);
        QVector4D midColor = m_rgbShader ?
                QVector4D(m_rgbMidColor_r, m_rgbMidColor_g, m_rgbMidColor_b, 1.0) :
                QVector4D(m_midColor_r, m_midColor_g, m_midColor_b, 1.0);
        QVector4D highColor = m_rgbShader ?
                QVector4D(m_rgbHighColor_r, m_rgbHighColor_g, m_rgbHighColor_b, 1.0) :
                QVector4D(m_highColor_r, m_highColor_g, m_highColor_b, 1.0);
        m_frameShaderProgram->setUniformValue("lowColor", lowColor);
        m_frameShaderProgram->setUniformValue("midColor", midColor);
        m_frameShaderProgram->setUniformValue("highColor", highColor);

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

    // paint buffer into viewport
    {
        // OpenGL pixels are real screen pixels, not device independent
        // pixels like QPainter provides. We scale the viewport by the
        // devicePixelRatio to render the texture to the surface.
        const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
        glViewport(0, 0, devicePixelRatio * m_waveformRenderer->getWidth(),
                   devicePixelRatio * m_waveformRenderer->getHeight());
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
