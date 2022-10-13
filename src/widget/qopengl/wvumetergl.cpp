#include "widget/qopengl/wvumetergl.h"

#include "util/math.h"
#include "util/timer.h"
#include "util/widgethelper.h"
#include "waveform/vsyncthread.h"
#include "widget/qopengl/moc_wvumetergl.cpp"
#include "widget/wpixmapstore.h"

#define DEFAULT_FALLTIME 20
#define DEFAULT_FALLSTEP 1
#define DEFAULT_HOLDTIME 400
#define DEFAULT_HOLDSIZE 5

WVuMeterGL::WVuMeterGL(QWidget* parent)
        : WGLWidget(parent),
          WBaseWidget(this),
          m_iRendersPending(0),
          m_bSwapNeeded(false),
          m_dParameter(0),
          m_dPeakParameter(0),
          m_dLastParameter(0),
          m_dLastPeakParameter(0),
          m_iPixmapLength(0),
          m_bHorizontal(false),
          m_iPeakHoldSize(0),
          m_iPeakFallStep(0),
          m_iPeakHoldTime(0),
          m_iPeakFallTime(0),
          m_dPeakHoldCountdownMs(0) {
}

WVuMeterGL::~WVuMeterGL() {
    makeCurrentIfNeeded();
    m_pTextureBack.reset();
    m_pTextureVu.reset();
    doneCurrent();
}

void WVuMeterGL::setup(const QDomNode& node, const SkinContext& context) {
    // Set pixmaps
    bool bHorizontal = false;
    (void)context.hasNodeSelectBool(node, "Horizontal", &bHorizontal);

    // Set background pixmap if available
    QDomElement backPathNode = context.selectElement(node, "PathBack");
    if (!backPathNode.isNull()) {
        // The implicit default in <1.12.0 was FIXED so we keep it for backwards
        // compatibility.
        setPixmapBackground(
                context.getPixmapSource(backPathNode),
                context.selectScaleMode(backPathNode, Paintable::FIXED),
                context.getScaleFactor());
    }

    QDomElement vuNode = context.selectElement(node, "PathVu");
    // The implicit default in <1.12.0 was FIXED so we keep it for backwards
    // compatibility.
    setPixmaps(context.getPixmapSource(vuNode),
            bHorizontal,
            context.selectScaleMode(vuNode, Paintable::FIXED),
            context.getScaleFactor());

    m_iPeakHoldSize = context.selectInt(node, "PeakHoldSize");
    if (m_iPeakHoldSize < 0 || m_iPeakHoldSize > 100) {
        m_iPeakHoldSize = DEFAULT_HOLDSIZE;
    }

    m_iPeakFallStep = context.selectInt(node, "PeakFallStep");
    if (m_iPeakFallStep < 1 || m_iPeakFallStep > 1000) {
        m_iPeakFallStep = DEFAULT_FALLSTEP;
    }

    m_iPeakHoldTime = context.selectInt(node, "PeakHoldTime");
    if (m_iPeakHoldTime < 1 || m_iPeakHoldTime > 3000) {
        m_iPeakHoldTime = DEFAULT_HOLDTIME;
    }

    m_iPeakFallTime = context.selectInt(node, "PeakFallTime");
    if (m_iPeakFallTime < 1 || m_iPeakFallTime > 1000) {
        m_iPeakFallTime = DEFAULT_FALLTIME;
    }

    setFocusPolicy(Qt::NoFocus);
}

void WVuMeterGL::setPixmapBackground(
        const PixmapSource& source,
        Paintable::DrawMode mode,
        double scaleFactor) {
    m_pPixmapBack = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (m_pPixmapBack.isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading background pixmap:" << source.getPath();
    } else if (mode == Paintable::FIXED) {
        setFixedSize(m_pPixmapBack->size());
    }
}

void WVuMeterGL::setPixmaps(const PixmapSource& source,
        bool bHorizontal,
        Paintable::DrawMode mode,
        double scaleFactor) {
    m_pPixmapVu = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (m_pPixmapVu.isNull()) {
        qDebug() << "WVuMeterGL: Error loading vu pixmap" << source.getPath();
    } else {
        m_bHorizontal = bHorizontal;
        if (m_bHorizontal) {
            m_iPixmapLength = m_pPixmapVu->width();
        } else {
            m_iPixmapLength = m_pPixmapVu->height();
        }
    }
}

void WVuMeterGL::onConnectedControlChanged(double dParameter, double dValue) {
    Q_UNUSED(dValue);
    m_dParameter = math_clamp(dParameter, 0.0, 1.0);

    if (dParameter > 0.0) {
        setPeak(dParameter);
    } else {
        // A 0.0 value is very unlikely except when the VU Meter is disabled
        m_dPeakParameter = 0;
    }
}

void WVuMeterGL::setPeak(double parameter) {
    if (parameter > m_dPeakParameter) {
        m_dPeakParameter = parameter;
        m_dPeakHoldCountdownMs = m_iPeakHoldTime;
    }
}

void WVuMeterGL::updateState(mixxx::Duration elapsed) {
    double msecsElapsed = elapsed.toDoubleMillis();
    // If we're holding at a peak then don't update anything
    m_dPeakHoldCountdownMs -= msecsElapsed;
    if (m_dPeakHoldCountdownMs > 0) {
        return;
    } else {
        m_dPeakHoldCountdownMs = 0;
    }

    // Otherwise, decrement the peak position by the fall step size times the
    // milliseconds elapsed over the fall time multiplier. The peak will fall
    // FallStep times (out of 128 steps) every FallTime milliseconds.
    m_dPeakParameter -= static_cast<double>(m_iPeakFallStep) *
            msecsElapsed /
            static_cast<double>(m_iPeakFallTime * m_iPixmapLength);
    m_dPeakParameter = math_clamp(m_dPeakParameter, 0.0, 1.0);
}

void WVuMeterGL::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
}

void WVuMeterGL::showEvent(QShowEvent* e) {
    Q_UNUSED(e);
    WGLWidget::showEvent(e);
    // Find the base color recursively in parent widget.
    m_qBgColor = mixxx::widgethelper::findBaseColor(this);
    // Force a rerender when the openglwindow is exposed.
    // 2 pendings renders, in case we have triple buffering
    m_iRendersPending = 2;
}

void WVuMeterGL::initializeGL() {
    if (m_pPixmapBack.isNull()) {
        m_pTextureBack.reset();
    } else {
        m_pTextureBack.reset(new QOpenGLTexture(m_pPixmapBack->toImage()));
        m_pTextureBack->setMinificationFilter(QOpenGLTexture::Linear);
        m_pTextureBack->setMagnificationFilter(QOpenGLTexture::Linear);
        m_pTextureBack->setWrapMode(QOpenGLTexture::ClampToBorder);
    }

    if (m_pPixmapVu.isNull()) {
        m_pTextureVu.reset();
    } else {
        m_pTextureVu.reset(new QOpenGLTexture(m_pPixmapVu->toImage()));
        m_pTextureVu->setMinificationFilter(QOpenGLTexture::Linear);
        m_pTextureVu->setMagnificationFilter(QOpenGLTexture::Linear);
        m_pTextureVu->setWrapMode(QOpenGLTexture::ClampToBorder);
    }

    QString vertexShaderCode =
            "\
uniform mat4 matrix;\n\
attribute vec4 position;\n\
attribute vec3 texcoor;\n\
varying vec3 vTexcoor;\n\
void main()\n\
{\n\
    vTexcoor = texcoor;\n\
    gl_Position = matrix * position;\n\
}\n";

    QString fragmentShaderCode =
            "\
uniform sampler2D sampler;\n\
varying vec3 vTexcoor;\n\
void main()\n\
{\n\
    gl_FragColor = texture2D(sampler, vec2(vTexcoor.x, vTexcoor.y));\n\
}\n";

    if (!m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderCode)) {
        return;
    }

    if (!m_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderCode)) {
        return;
    }

    if (!m_shaderProgram.link()) {
        return;
    }

    if (!m_shaderProgram.bind()) {
        return;
    }
}

void WVuMeterGL::render(VSyncThread* vSyncThread) {
    ScopedTimer t("WVuMeterGL::render");

    updateState(vSyncThread->sinceLastSwap());

    if (m_dParameter != m_dLastParameter || m_dPeakParameter != m_dLastPeakParameter) {
        m_iRendersPending = 2;
    }

    if (m_iRendersPending == 0 || !shouldRender()) {
        return;
    }

    makeCurrentIfNeeded();
    drawNativeGL();
    doneCurrent();
}

void WVuMeterGL::drawNativeGL() {
    glClearColor(m_qBgColor.redF(), m_qBgColor.greenF(), m_qBgColor.blueF(), 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_shaderProgram.bind();

    QMatrix4x4 matrix;
    matrix.ortho(QRectF(0, 0, width(), height()));

    int matrixLocation = m_shaderProgram.uniformLocation("matrix");

    m_shaderProgram.setUniformValue(matrixLocation, matrix);

    if (m_pTextureBack) {
        // Draw background.
        QRectF sourceRect(0, 0, m_pPixmapBack->width(), m_pPixmapBack->height());
        drawTexture(m_pTextureBack.get(), rect(), sourceRect);
    }

    const double widgetWidth = width();
    const double widgetHeight = height();

    if (m_pTextureVu) {
        const double pixmapWidth = m_pTextureVu->width();
        const double pixmapHeight = m_pTextureVu->height();
        if (m_bHorizontal) {
            const double widgetPosition = math_clamp(widgetWidth * m_dParameter, 0.0, widgetWidth);
            QRectF targetRect(0, 0, widgetPosition, widgetHeight);

            const double pixmapPosition = math_clamp(
                    pixmapWidth * m_dParameter, 0.0, pixmapWidth);
            QRectF sourceRect(0, 0, pixmapPosition, pixmapHeight);
            drawTexture(m_pTextureVu.get(), targetRect, sourceRect);

            if (m_iPeakHoldSize > 0 && m_dPeakParameter > 0.0 &&
                    m_dPeakParameter > m_dParameter) {
                const double widgetPeakPosition = math_clamp(
                        widgetWidth * m_dPeakParameter, 0.0, widgetWidth);
                const double pixmapPeakHoldSize = static_cast<double>(m_iPeakHoldSize);
                const double widgetPeakHoldSize = widgetWidth * pixmapPeakHoldSize / pixmapWidth;

                QRectF targetRect(widgetPeakPosition - widgetPeakHoldSize,
                        0,
                        widgetPeakHoldSize,
                        widgetHeight);

                const double pixmapPeakPosition = math_clamp(
                        pixmapWidth * m_dPeakParameter, 0.0, pixmapWidth);

                QRectF sourceRect =
                        QRectF(pixmapPeakPosition - pixmapPeakHoldSize,
                                0,
                                pixmapPeakHoldSize,
                                pixmapHeight);
                drawTexture(m_pTextureVu.get(), targetRect, sourceRect);
            }
        } else {
            // vertical
            const double widgetPosition =
                    math_clamp(widgetHeight * m_dParameter, 0.0, widgetHeight);
            QRectF targetRect(0, widgetHeight - widgetPosition, widgetWidth, widgetPosition);

            const double pixmapPosition = math_clamp(
                    pixmapHeight * m_dParameter, 0.0, pixmapHeight);
            QRectF sourceRect(0, pixmapHeight - pixmapPosition, pixmapWidth, pixmapPosition);
            drawTexture(m_pTextureVu.get(), targetRect, sourceRect);

            if (m_iPeakHoldSize > 0 && m_dPeakParameter > 0.0 &&
                    m_dPeakParameter > m_dParameter) {
                const double widgetPeakPosition = math_clamp(
                        widgetHeight * m_dPeakParameter, 0.0, widgetHeight);
                const double pixmapPeakHoldSize = static_cast<double>(m_iPeakHoldSize);
                const double widgetPeakHoldSize = widgetHeight * pixmapPeakHoldSize / pixmapHeight;

                QRectF targetRect(0,
                        widgetHeight - widgetPeakPosition,
                        widgetWidth,
                        widgetPeakHoldSize);

                const double pixmapPeakPosition = math_clamp(
                        pixmapHeight * m_dPeakParameter, 0.0, pixmapHeight);

                QRectF sourceRect = QRectF(0,
                        pixmapHeight - pixmapPeakPosition,
                        pixmapWidth,
                        pixmapPeakHoldSize);
                drawTexture(m_pTextureVu.get(), targetRect, sourceRect);
            }
        }
    }

    m_dLastParameter = m_dParameter;
    m_dLastPeakParameter = m_dPeakParameter;
    m_iRendersPending--;
    m_bSwapNeeded = true;
}

void WVuMeterGL::swap() {
    // TODO @m0dB move shouldRender outside?
    if (!m_bSwapNeeded || !shouldRender()) {
        return;
    }
    makeCurrentIfNeeded();
    swapBuffers();
    doneCurrent();
    m_bSwapNeeded = false;
}

void WVuMeterGL::drawTexture(QOpenGLTexture* texture,
        const QRectF& targetRect,
        const QRectF& sourceRect) {
    const float texx1 = sourceRect.x() / texture->width();
    const float texy1 = sourceRect.y() / texture->height();
    const float texx2 = (sourceRect.x() + sourceRect.width()) / texture->width();
    const float texy2 = (sourceRect.y() + sourceRect.height()) / texture->height();

    const float posx1 = targetRect.x();
    const float posy1 = targetRect.y();
    const float posx2 = targetRect.x() + targetRect.width();
    const float posy2 = targetRect.y() + targetRect.height();

    const float posarray[] = {posx1, posy1, posx2, posy1, posx1, posy2, posx2, posy2};
    const float texarray[] = {texx1, texy1, texx2, texy1, texx1, texy2, texx2, texy2};

    int samplerLocation = m_shaderProgram.uniformLocation("sampler");
    int positionLocation = m_shaderProgram.attributeLocation("position");
    int texcoordLocation = m_shaderProgram.attributeLocation("texcoor");

    m_shaderProgram.enableAttributeArray(positionLocation);
    m_shaderProgram.setAttributeArray(
            positionLocation, GL_FLOAT, posarray, 2);
    m_shaderProgram.enableAttributeArray(texcoordLocation);
    m_shaderProgram.setAttributeArray(
            texcoordLocation, GL_FLOAT, texarray, 2);

    m_shaderProgram.setUniformValue(samplerLocation, 0);

    texture->bind();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
