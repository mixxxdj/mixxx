#include "widget/wvumeterglsl.h"

#include <QOpenGLTexture>

#include "moc_wvumeterglsl.cpp"
#include "util/duration.h"
#include "util/math.h"
#include "util/texture.h"

WVuMeterGLSL::WVuMeterGLSL(QWidget* pParent)
        : WVuMeterBase(pParent) {
}

WVuMeterGLSL::~WVuMeterGLSL() {
    cleanupGL();
}

void WVuMeterGLSL::draw() {
    if (shouldRender()) {
        makeCurrentIfNeeded();
        paintGL();
        doneCurrent();
    }
}

void WVuMeterGLSL::initializeGL() {
    initializeOpenGLFunctions();

    m_pTextureBack = createTexture(m_pPixmapBack);
    m_pTextureVu = createTexture(m_pPixmapVu);
    m_textureShader.init();
}

void WVuMeterGLSL::paintGL() {
    glClearColor(static_cast<float>(m_qBgColor.redF()),
            static_cast<float>(m_qBgColor.greenF()),
            static_cast<float>(m_qBgColor.blueF()),
            1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    QMatrix4x4 matrix;
    matrix.ortho(QRectF(0, 0, width(), height()));

    m_textureShader.bind();

    int matrixLocation = m_textureShader.matrixLocation();
    int textureLocation = m_textureShader.textureLocation();
    int positionLocation = m_textureShader.positionLocation();
    int texcoordLocation = m_textureShader.texcoordLocation();

    m_textureShader.setUniformValue(matrixLocation, matrix);

    m_textureShader.setUniformValue(matrixLocation, matrix);

    m_textureShader.enableAttributeArray(positionLocation);
    m_textureShader.enableAttributeArray(texcoordLocation);

    m_textureShader.setUniformValue(textureLocation, 0);

    if (m_pTextureBack) {
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
            QRectF targetRect(0, 0, widgetPosition, math_min(pixmapHeight, widgetHeight));

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
                        math_min(pixmapHeight, widgetHeight));

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

    m_textureShader.disableAttributeArray(positionLocation);
    m_textureShader.disableAttributeArray(texcoordLocation);
    m_textureShader.release();
}

void WVuMeterGLSL::cleanupGL() {
    makeCurrentIfNeeded();
    m_pTextureBack.reset();
    m_pTextureVu.reset();
    doneCurrent();
}

void WVuMeterGLSL::drawTexture(QOpenGLTexture* texture,
        const QRectF& targetRect,
        const QRectF& sourceRect) {
    const float texx1 = static_cast<float>(sourceRect.x() / texture->width());
    const float texy1 = static_cast<float>(sourceRect.y() / texture->height());
    const float texx2 = static_cast<float>(
            (sourceRect.x() + sourceRect.width()) / texture->width());
    const float texy2 = static_cast<float>(
            (sourceRect.y() + sourceRect.height()) / texture->height());

    const float posx1 = static_cast<float>(targetRect.x());
    const float posy1 = static_cast<float>(targetRect.y());
    const float posx2 = static_cast<float>(targetRect.x() + targetRect.width());
    const float posy2 = static_cast<float>(targetRect.y() + targetRect.height());

    const std::array<float, 8> posarray = {posx1, posy1, posx2, posy1, posx1, posy2, posx2, posy2};
    const std::array<float, 8> texarray = {texx1, texy1, texx2, texy1, texx1, texy2, texx2, texy2};

    int positionLocation = m_textureShader.positionLocation();
    int texcoordLocation = m_textureShader.texcoordLocation();

    m_textureShader.setAttributeArray(
            positionLocation, GL_FLOAT, posarray.data(), 2);
    m_textureShader.setAttributeArray(
            texcoordLocation, GL_FLOAT, texarray.data(), 2);

    texture->bind();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    texture->release();
}
