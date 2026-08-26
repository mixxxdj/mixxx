#include "waveform/renderers/allshader/waveformrenderertextured.h"

#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include "moc_waveformrenderertextured.cpp"
#include "track/track.h"
#include "util/math.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"

namespace allshader {

// static
QString WaveformRendererTextured::fragShaderForType(WaveformRendererTextured::Type t) {
    using Type = WaveformRendererTextured::Type;
    switch (t) {
    case Type::Filtered:
        return QLatin1String(":/shaders/texturedfilteredsignal.frag");
    case Type::RGB:
        return QLatin1String(":/shaders/texturedrgbsignal.frag");
    case Type::Stacked:
        return QLatin1String(":/shaders/texturedstackedsignal.frag");
    default:
        break;
    }
    DEBUG_ASSERT(!"unsupported WaveformWidgetType");
    return QString();
}

WaveformRendererTextured::WaveformRendererTextured(WaveformWidgetRenderer* waveformWidget,
        Type t)
        : WaveformRendererSignalBase(waveformWidget),
          m_textureId(0),
          m_textureRenderedWaveformCompletion(0),
          m_shadersValid(false),
          m_type(t),
          m_pFragShader(fragShaderForType(t)),
          m_matrixLocation(-1),
          m_positionLocation(-1),
          m_texcoordLocation(-1) {
}

WaveformRendererTextured::~WaveformRendererTextured() {
    if (m_textureId) {
        glDeleteTextures(1, &m_textureId);
    }

    if (m_frameShaderProgram) {
        m_frameShaderProgram->removeAllShaders();
    }
}

bool WaveformRendererTextured::loadShaders() {
    qDebug() << "WaveformRendererTextured::loadShaders";
    m_shadersValid = false;

    if (m_frameShaderProgram->isLinked()) {
        m_frameShaderProgram->release();
    }

    m_frameShaderProgram->removeAllShaders();

    if (!m_frameShaderProgram->addShaderFromSourceFile(
                QOpenGLShader::Vertex,
                ":/shaders/texturedsignalpassthrough.vert")) {
        qDebug() << "WaveformRendererTextured::loadShaders - "
                 << m_frameShaderProgram->log();
        return false;
    }

    if (!m_frameShaderProgram->addShaderFromSourceFile(
                QOpenGLShader::Fragment,
                m_pFragShader)) {
        qDebug() << "WaveformRendererTextured::loadShaders - "
                 << m_frameShaderProgram->log();
        return false;
    }

    if (!m_frameShaderProgram->link()) {
        qDebug() << "WaveformRendererTextured::loadShaders - "
                 << m_frameShaderProgram->log();
        return false;
    }

    if (!m_frameShaderProgram->bind()) {
        qDebug() << "WaveformRendererTextured::loadShaders - shaders binding failed";
        return false;
    }

    m_matrixLocation = m_frameShaderProgram->uniformLocation("matrix");
    m_positionLocation = m_frameShaderProgram->attributeLocation("position");
    m_texcoordLocation = m_frameShaderProgram->attributeLocation("texcoord");

    m_shadersValid = true;
    return true;
}

bool WaveformRendererTextured::loadTexture() {
    int dataSize = 0;
    const WaveformData* data = nullptr;

    ConstWaveformPointer pWaveform = m_waveformRenderer->getWaveform();
    if (pWaveform) {
        dataSize = pWaveform->getDataSize();
        if (dataSize > 1) {
            data = pWaveform->data();
        }
    }

    if (m_textureId == 0) {
        glGenTextures(1, &m_textureId);

        int error = glGetError();
        if (error) {
            qDebug() << "WaveformRendererTextured::loadTexture - m_textureId"
                     << m_textureId << "error" << error;
        }
    }

    glBindTexture(GL_TEXTURE_2D, m_textureId);

    int error = glGetError();
    if (error) {
        qDebug() << "WaveformRendererTextured::loadTexture - bind error" << error;
    }

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    if (pWaveform != nullptr && data != nullptr) {
        // Waveform ensures that getTextureSize is a multiple of
        // getTextureStride so there is no rounding here.
        int textureWidth = pWaveform->getTextureStride();
        int textureHeight = pWaveform->getTextureSize() / pWaveform->getTextureStride();

        glTexImage2D(GL_TEXTURE_2D,
                0,
                GL_RGBA,
                textureWidth,
                textureHeight,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                data);
        int error = glGetError();
        if (error) {
            qDebug() << "WaveformRendererTextured::loadTexture - glTexImage2D error" << error;
        }
    } else {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
    }

    return true;
}

void WaveformRendererTextured::createFrameBuffers() {
    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    const int baseWidth = static_cast<int>(m_waveformRenderer->getWidth() * devicePixelRatio);
    const int baseHeight = static_cast<int>(m_waveformRenderer->getHeight() * devicePixelRatio);

    // We create a frame buffer that is up to 4x the size of the renderer
    // itself to "oversample" the texture relative to the surface we're
    // drawing on. On a very wide and/or high-DPI surface (e.g. an ultrawide
    // monitor in fullscreen), naively multiplying by a fixed factor of 4 can
    // request a framebuffer larger than the GPU's maximum texture size. That
    // silently produces an invalid framebuffer, which used to be logged and
    // then used anyway, rendering a blank waveform. Back off the
    // oversampling factor as needed to fit within the GPU's limit.
    //
    // Query through QOpenGLContext::functions() rather than calling
    // glGetIntegerv() directly: at this point in the renderer's lifecycle
    // the context is valid but is not necessarily current via the classic
    // WGL/opengl32.dll thread-local state that a raw call relies on, and
    // Qt's function table is the safe way to reach it regardless.
    GLint maxTextureSize = 0;
    if (QOpenGLContext* context = QOpenGLContext::currentContext()) {
        context->functions()->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    }

    constexpr int oversamplingFactor = 4;
    const int bufferWidth = maxTextureSize > 0
            ? math_min(baseWidth * oversamplingFactor, maxTextureSize)
            : baseWidth * oversamplingFactor;
    const int bufferHeight = maxTextureSize > 0
            ? math_min(baseHeight * oversamplingFactor, maxTextureSize)
            : baseHeight * oversamplingFactor;

    m_framebuffer = std::make_unique<QOpenGLFramebufferObject>(bufferWidth,
            bufferHeight);

    if (!m_framebuffer->isValid()) {
        qWarning() << "WaveformRendererTextured::createFrameBuffer - frame buffer not valid"
                   << "requested size" << bufferWidth << "x" << bufferHeight
                   << "GL_MAX_TEXTURE_SIZE" << maxTextureSize;
        m_framebuffer.reset();
    }
}

void WaveformRendererTextured::initializeGL() {
    WaveformRendererSignalBase::initializeGL();

    m_textureRenderedWaveformCompletion = 0;

    if (!m_frameShaderProgram) {
        m_frameShaderProgram = std::make_unique<QOpenGLShaderProgram>();
    }

    if (!loadShaders()) {
        return;
    }
    m_textureShader.init();
    createFrameBuffers();
    if (!loadTexture()) {
        return;
    }
}

void WaveformRendererTextured::onSetup(const QDomNode& node) {
    Q_UNUSED(node);
}

void WaveformRendererTextured::onSetTrack() {
    if (m_loadedTrack) {
        disconnect(m_loadedTrack.get(),
                &Track::waveformUpdated,
                this,
                &WaveformRendererTextured::slotWaveformUpdated);
    }

    slotWaveformUpdated();

    const TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        return;
    }

    // When the track's waveform has been changed (or cleared), it is necessary
    // to update (or delete) the texture containing the waveform which was
    // uploaded to GPU. Otherwise, previous waveform will be shown.
    connect(pTrack.get(),
            &Track::waveformUpdated,
            this,
            &WaveformRendererTextured::slotWaveformUpdated);

    m_loadedTrack = pTrack;
}

void WaveformRendererTextured::resizeGL(int, int) {
    createFrameBuffers();
}

void WaveformRendererTextured::slotWaveformUpdated() {
    m_textureRenderedWaveformCompletion = 0;
    // initializeGL not called yet
    if (!m_frameShaderProgram) {
        return;
    }
    loadTexture();
}

void WaveformRendererTextured::paintGL() {
    ConstWaveformPointer pWaveform = m_waveformRenderer->getWaveform();
    if (pWaveform.isNull()) {
        return;
    }

    const double audioVisualRatio = pWaveform->getAudioVisualRatio();
    if (audioVisualRatio <= 0) {
        return;
    }

    int dataSize = pWaveform->getDataSize();
    if (dataSize <= 1) {
        return;
    }

    const WaveformData* data = pWaveform->data();
    if (data == nullptr) {
        return;
    }

    const double trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0) {
        return;
    }

    if (!m_framebuffer || !m_framebuffer->isValid()) {
        // The framebuffer failed to allocate (e.g. the requested oversampled
        // size exceeded the GPU's maximum texture size). Skip drawing rather
        // than blitting an invalid/blank texture.
        return;
    }

    // NOTE(vRince): completion can change during loadTexture
    // do not remove currenCompletion temp variable !
    const int currentCompletion = pWaveform->getCompletion();
    if (m_textureRenderedWaveformCompletion < currentCompletion) {
        loadTexture();
        m_textureRenderedWaveformCompletion = currentCompletion;
    }

    // Per-band gain from the EQ knobs.
    float lowGain(1.0), midGain(1.0), highGain(1.0), allGain(1.0);
    getGains(&allGain, true, &lowGain, &midGain, &highGain);

    const auto firstVisualIndex = static_cast<GLfloat>(
            m_waveformRenderer->getFirstDisplayedPosition() * trackSamples /
            audioVisualRatio / 2.0);
    const auto lastVisualIndex = static_cast<GLfloat>(
            m_waveformRenderer->getLastDisplayedPosition() * trackSamples / audioVisualRatio / 2.0);

    // const int firstIndex = int(firstVisualIndex+0.5);
    // firstVisualIndex = firstIndex - firstIndex%2;

    // const int lastIndex = int(lastVisualIndex+0.5);
    // lastVisualIndex = lastIndex + lastIndex%2;

    // qDebug() << "GAIN" << allGain << lowGain << midGain << highGain;

    // paint into frame buffer
    {
        QMatrix4x4 matrix;
        if (m_orientation == Qt::Vertical) {
            matrix.rotate(90.0f, 0.0f, 0.0f, 1.0f);
            matrix.scale(-1.0f, 1.0f, 1.0f);
        }
        matrix.ortho(firstVisualIndex, lastVisualIndex, -1.0f, 1.0f, -10.0f, 10.0f);

        m_frameShaderProgram->bind();

        glViewport(0, 0, m_framebuffer->width(), m_framebuffer->height());

        m_frameShaderProgram->setUniformValue(m_matrixLocation, matrix);

        m_frameShaderProgram->setUniformValue("framebufferSize",
                QVector2D(m_framebuffer->width(), m_framebuffer->height()));
        m_frameShaderProgram->setUniformValue("waveformLength", dataSize);
        m_frameShaderProgram->setUniformValue("textureSize", pWaveform->getTextureSize());
        m_frameShaderProgram->setUniformValue("textureStride", pWaveform->getTextureStride());

        m_frameShaderProgram->setUniformValue("firstVisualIndex", firstVisualIndex);
        m_frameShaderProgram->setUniformValue("lastVisualIndex", lastVisualIndex);

        m_frameShaderProgram->setUniformValue("allGain", allGain);
        m_frameShaderProgram->setUniformValue("lowGain", lowGain);
        m_frameShaderProgram->setUniformValue("midGain", midGain);
        m_frameShaderProgram->setUniformValue("highGain", highGain);

        m_frameShaderProgram->setUniformValue("axesColor",
                QVector4D(static_cast<GLfloat>(m_axesColor_r),
                        static_cast<GLfloat>(m_axesColor_g),
                        static_cast<GLfloat>(m_axesColor_b),
                        static_cast<GLfloat>(m_axesColor_a)));

        if (m_type == Type::Stacked) {
            m_frameShaderProgram->setUniformValue("lowFilteredColor",
                    QVector4D(static_cast<GLfloat>(m_rgbLowFilteredColor_r),
                            static_cast<GLfloat>(m_rgbLowFilteredColor_g),
                            static_cast<GLfloat>(m_rgbLowFilteredColor_b),
                            1.0));
            m_frameShaderProgram->setUniformValue("midFilteredColor",
                    QVector4D(static_cast<GLfloat>(m_rgbMidFilteredColor_r),
                            static_cast<GLfloat>(m_rgbMidFilteredColor_g),
                            static_cast<GLfloat>(m_rgbMidFilteredColor_b),
                            1.0));
            m_frameShaderProgram->setUniformValue("highFilteredColor",
                    QVector4D(static_cast<GLfloat>(m_rgbHighFilteredColor_r),
                            static_cast<GLfloat>(m_rgbHighFilteredColor_g),
                            static_cast<GLfloat>(m_rgbHighFilteredColor_b),
                            1.0));
        }
        if (m_type == Type::RGB || m_type == Type::Stacked) {
            m_frameShaderProgram->setUniformValue("lowColor",
                    QVector4D(static_cast<GLfloat>(m_rgbLowColor_r),
                            static_cast<GLfloat>(m_rgbLowColor_g),
                            static_cast<GLfloat>(m_rgbLowColor_b),
                            1.0));
            m_frameShaderProgram->setUniformValue("midColor",
                    QVector4D(static_cast<GLfloat>(m_rgbMidColor_r),
                            static_cast<GLfloat>(m_rgbMidColor_g),
                            static_cast<GLfloat>(m_rgbMidColor_b),
                            1.0));
            m_frameShaderProgram->setUniformValue("highColor",
                    QVector4D(static_cast<GLfloat>(m_rgbHighColor_r),
                            static_cast<GLfloat>(m_rgbHighColor_g),
                            static_cast<GLfloat>(m_rgbHighColor_b),
                            1.0));
        } else {
            m_frameShaderProgram->setUniformValue("lowColor",
                    QVector4D(static_cast<GLfloat>(m_lowColor_r),
                            static_cast<GLfloat>(m_lowColor_g),
                            static_cast<GLfloat>(m_lowColor_b),
                            1.0));
            m_frameShaderProgram->setUniformValue("midColor",
                    QVector4D(static_cast<GLfloat>(m_midColor_r),
                            static_cast<GLfloat>(m_midColor_g),
                            static_cast<GLfloat>(m_midColor_b),
                            1.0));
            m_frameShaderProgram->setUniformValue("highColor",
                    QVector4D(static_cast<GLfloat>(m_highColor_r),
                            static_cast<GLfloat>(m_highColor_g),
                            static_cast<GLfloat>(m_highColor_b),
                            1.0));
        }

        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        m_framebuffer->bind();
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Quad spanning [firstVisualIndex, lastVisualIndex] x [-1, 1], in
        // (bottom-left, bottom-right, top-left, top-right) order to draw as
        // a triangle strip.
        const float posarray[] = {
                firstVisualIndex,
                -1.0f,
                lastVisualIndex,
                -1.0f,
                firstVisualIndex,
                1.0f,
                lastVisualIndex,
                1.0f,
        };
        const float texarray[] = {
                0.0f,
                0.0f,
                1.0f,
                0.0f,
                0.0f,
                1.0f,
                1.0f,
                1.0f,
        };

        m_frameShaderProgram->enableAttributeArray(m_positionLocation);
        m_frameShaderProgram->setAttributeArray(
                m_positionLocation, GL_FLOAT, posarray, 2);
        m_frameShaderProgram->enableAttributeArray(m_texcoordLocation);
        m_frameShaderProgram->setAttributeArray(
                m_texcoordLocation, GL_FLOAT, texarray, 2);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        m_frameShaderProgram->disableAttributeArray(m_positionLocation);
        m_frameShaderProgram->disableAttributeArray(m_texcoordLocation);

        m_framebuffer->release();

        m_frameShaderProgram->release();
    }

    // paint buffer into viewport
    {
        // OpenGL pixels are real screen pixels, not device independent
        // pixels like QPainter provides. We scale the viewport by the
        // devicePixelRatio to render the texture to the surface.
        const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
        glViewport(0,
                0,
                static_cast<GLsizei>(
                        devicePixelRatio * m_waveformRenderer->getWidth()),
                static_cast<GLsizei>(
                        devicePixelRatio * m_waveformRenderer->getHeight()));

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Fullscreen quad in NDC, so an identity matrix suffices.
        const QMatrix4x4 matrix;

        const float posarray[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};
        const float texarray[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};

        m_textureShader.bind();

        const int matrixLocation = m_textureShader.matrixLocation();
        const int textureLocation = m_textureShader.textureLocation();
        const int positionLocation = m_textureShader.positionLocation();
        const int texcoordLocation = m_textureShader.texcoordLocation();

        m_textureShader.setUniformValue(matrixLocation, matrix);
        m_textureShader.setUniformValue(textureLocation, 0);

        m_textureShader.enableAttributeArray(positionLocation);
        m_textureShader.setAttributeArray(positionLocation, GL_FLOAT, posarray, 2);
        m_textureShader.enableAttributeArray(texcoordLocation);
        m_textureShader.setAttributeArray(texcoordLocation, GL_FLOAT, texarray, 2);

        glBindTexture(GL_TEXTURE_2D, m_framebuffer->texture());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        m_textureShader.disableAttributeArray(positionLocation);
        m_textureShader.disableAttributeArray(texcoordLocation);
        m_textureShader.release();

        // Other renderers sharing this widget (e.g. cue/hotcue marks) bind
        // their own textures through QOpenGLTexture::bind(), which caches
        // the last-bound texture ID per unit and can skip the real
        // glBindTexture() call if it believes its texture is already
        // current. Our raw glBindTexture() calls above bypass that cache
        // entirely, leaving it out of sync with the GL state we actually
        // set. Explicitly unbinding here guarantees the next renderer's
        // QOpenGLTexture::bind() call always performs a real state change
        // instead of silently no-op'ing against stale bookkeeping.
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

} // namespace allshader
