#include "widget/wvumetergl.h"

#include "util/math.h"

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

    QString vertexShaderCode = QStringLiteral(R"--(
uniform mat4 matrix;
attribute vec4 position;
attribute vec3 texcoor;
varying vec3 vTexcoor;
void main()
{
    vTexcoor = texcoor;
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
uniform sampler2D sampler;
varying vec3 vTexcoor;
void main()
{
    gl_FragColor = texture2D(sampler, vec2(vTexcoor.x, vTexcoor.y));
}
)--");

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

void WVuMeterGL::renderGL() {
    if (!shouldRender()) {
        return;
    }

    makeCurrentIfNeeded();
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

    doneCurrent();
}

void WVuMeterGL::cleanupGL() {
    makeCurrentIfNeeded();
    m_pTextureBack.reset();
    m_pTextureVu.reset();
    doneCurrent();
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
