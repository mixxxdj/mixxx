#include "waveform/renderers/allshader/waveformrendermark.h"

#include <QDomNode>
#include <QOpenGLTexture>
#include <QPainterPath>

#include "control/controlobject.h"
#include "engine/controls/cuecontrol.h"
#include "track/track.h"
#include "util/color/color.h"
#include "util/colorcomponents.h"
#include "util/painterscope.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/renderers/allshader/moc_waveformrendermark.cpp"
#include "waveform/renderers/allshader/rgbadata.h"
#include "waveform/renderers/allshader/vertexdata.h"
#include "waveform/waveform.h"
#include "waveform/widgets/allshader/waveformwidget.h"
#include "widget/wimagestore.h"
#include "widget/wskincolor.h"

// On the use of QPainter:
//
// The renderers in this folder are optimized to use GLSL shaders and refrain
// from using QPainter on the QOpenGLWindow, which causes degredated performance.
//
// This renderer does use QPainter, but only to draw on a QImage. This is only
// done once when needed and the images are then used as textures to be drawn
// with a GLSL shader.

namespace {
constexpr int kMaxCueLabelLength = 23;
} // namespace

allshader::WaveformRenderMark::WaveformRenderMark(WaveformWidgetRenderer* waveformWidget)
        : WaveformRenderer(waveformWidget) {
}

allshader::WaveformRenderMark::~WaveformRenderMark() {
    for (const auto& pMark : m_marks) {
        pMark->m_pTexture.reset();
    }
}

void allshader::WaveformRenderMark::setup(const QDomNode& node, const SkinContext& context) {
    WaveformSignalColors signalColors = *m_waveformRenderer->getWaveformSignalColors();
    m_marks.setup(m_waveformRenderer->getGroup(), node, context, signalColors);
}

void allshader::WaveformRenderMark::initializeGL() {
    WaveformRenderer::initializeGL();
    m_rgbaShader.init();
    m_textureShader.init();

    for (const auto& pMark : m_marks) {
        generateMarkImage(pMark, m_waveformRenderer->getBreadth());
    }
    generatePlayPosMarkTexture(m_waveformRenderer->getBreadth());
}

void allshader::WaveformRenderMark::drawTexture(float x, float y, QOpenGLTexture* texture) {
    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    const float texx1 = 0.f;
    const float texy1 = 0.f;
    const float texx2 = 1.f;
    const float texy2 = 1.f;

    const float posx1 = x;
    const float posx2 = x + static_cast<float>(texture->width() / devicePixelRatio);
    const float posy1 = y;
    const float posy2 = y + static_cast<float>(texture->height() / devicePixelRatio);

    const float posarray[] = {posx1, posy1, posx2, posy1, posx1, posy2, posx2, posy2};
    const float texarray[] = {texx1, texy1, texx2, texy1, texx1, texy2, texx2, texy2};

    const QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, false);

    m_textureShader.bind();

    int matrixLocation = m_textureShader.uniformLocation("matrix");
    int samplerLocation = m_textureShader.uniformLocation("sampler");
    int positionLocation = m_textureShader.attributeLocation("position");
    int texcoordLocation = m_textureShader.attributeLocation("texcoor");

    m_textureShader.setUniformValue(matrixLocation, matrix);

    m_textureShader.enableAttributeArray(positionLocation);
    m_textureShader.setAttributeArray(
            positionLocation, GL_FLOAT, posarray, 2);
    m_textureShader.enableAttributeArray(texcoordLocation);
    m_textureShader.setAttributeArray(
            texcoordLocation, GL_FLOAT, texarray, 2);

    m_textureShader.setUniformValue(samplerLocation, 0);

    texture->bind();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    texture->release();

    m_textureShader.disableAttributeArray(positionLocation);
    m_textureShader.disableAttributeArray(texcoordLocation);
    m_textureShader.release();
}

void allshader::WaveformRenderMark::drawMark(const QRectF& rect, QColor color) {
    // draw a gradient towards transparency at the upper and lower 25% of the waveform view

    const float qh = static_cast<float>(std::floor(rect.height() * 0.25));
    const float posx1 = static_cast<float>(rect.x());
    const float posx2 = static_cast<float>(rect.x() + rect.width());
    const float posy1 = static_cast<float>(rect.y());
    const float posy2 = static_cast<float>(rect.y()) + qh;
    const float posy3 = static_cast<float>(rect.y() + rect.height()) - qh;
    const float posy4 = static_cast<float>(rect.y() + rect.height());

    float r, g, b, a;

    getRgbF(color, &r, &g, &b, &a);

    VertexData vertices;
    vertices.reserve(12); // 4 triangles
    vertices.addRectangle(posx1, posy1, posx2, posy2);
    vertices.addRectangle(posx1, posy4, posx2, posy3);

    RGBAData rgbaData;
    rgbaData.reserve(12); // 4 triangles
    rgbaData.addForRectangleGradient(r, g, b, a, r, g, b, 0.f);
    rgbaData.addForRectangleGradient(r, g, b, a, r, g, b, 0.f);

    QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, false);

    m_rgbaShader.bind();

    int matrixLocation = m_rgbaShader.uniformLocation("matrix");
    int positionLocation = m_rgbaShader.attributeLocation("position");
    int colorLocation = m_rgbaShader.attributeLocation("color");

    m_rgbaShader.setUniformValue(matrixLocation, matrix);

    m_rgbaShader.enableAttributeArray(positionLocation);
    m_rgbaShader.setAttributeArray(
            positionLocation, GL_FLOAT, vertices.constData(), 2);
    m_rgbaShader.enableAttributeArray(colorLocation);
    m_rgbaShader.setAttributeArray(
            colorLocation, GL_FLOAT, rgbaData.constData(), 4);

    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    m_rgbaShader.disableAttributeArray(positionLocation);
    m_rgbaShader.disableAttributeArray(colorLocation);
    m_rgbaShader.release();
}

void allshader::WaveformRenderMark::paintGL() {
    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    QMap<WaveformMarkPointer, int> marksOnScreen;

    checkCuesUpdated();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const auto& pMark : m_marks) {
        if (!pMark->isValid()) {
            continue;
        }

        if (pMark->hasVisible() && !pMark->isVisible()) {
            continue;
        }

        if (pMark->m_image.isNull()) {
            generateMarkImage(pMark, m_waveformRenderer->getBreadth());
        }

        const double samplePosition = pMark->getSamplePosition();
        if (samplePosition != Cue::kNoPosition) {
            const float currentMarkPoint = std::floor(static_cast<float>(
                    m_waveformRenderer->transformSamplePositionInRendererWorld(
                            samplePosition)));
            const double sampleEndPosition = pMark->getSampleEndPosition();

            // Pixmaps are expected to have the mark stroke at the center,
            // and preferably have an odd width in order to have the stroke
            // exactly at the sample position.
            const float markHalfWidth = pMark->m_pTexture->width() / devicePixelRatio / 2.f;
            const float drawOffset = currentMarkPoint - markHalfWidth;

            bool visible = false;
            // Check if the current point needs to be displayed.
            if (currentMarkPoint > -markHalfWidth &&
                    currentMarkPoint < m_waveformRenderer->getLength() +
                                    markHalfWidth) {
                drawTexture(drawOffset, 0, pMark->m_pTexture.get());
                visible = true;
            }

            // Check if the range needs to be displayed.
            if (samplePosition != sampleEndPosition && sampleEndPosition != Cue::kNoPosition) {
                DEBUG_ASSERT(samplePosition < sampleEndPosition);
                const float currentMarkEndPoint = std::floor(static_cast<
                        float>(
                        m_waveformRenderer
                                ->transformSamplePositionInRendererWorld(
                                        sampleEndPosition)));

                if (visible || currentMarkEndPoint > 0) {
                    QColor color = pMark->fillColor();
                    color.setAlphaF(0.4);

                    drawMark(
                            QRectF(QPointF(currentMarkPoint, 0),
                                    QPointF(currentMarkEndPoint,
                                            m_waveformRenderer
                                                    ->getBreadth())),
                            color);
                    visible = true;
                }
            }

            if (visible) {
                marksOnScreen[pMark] = static_cast<int>(drawOffset);
            }
        }
    }
    m_waveformRenderer->setMarkPositions(marksOnScreen);

    const float currentMarkPoint = std::floor(
            static_cast<float>(m_waveformRenderer->getPlayMarkerPosition() *
                    m_waveformRenderer->getLength()));

    const float markHalfWidth = m_pPlayPosMarkTexture->width() / devicePixelRatio / 2.f;
    const float drawOffset = currentMarkPoint - markHalfWidth;

    drawTexture(drawOffset, 0.f, m_pPlayPosMarkTexture.get());
}

void allshader::WaveformRenderMark::generatePlayPosMarkTexture(float breadth) {
    float imgwidth;
    float imgheight;

    const float height = breadth;
    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();

    const float lineX = 5.5f;

    imgwidth = 11.f;
    imgheight = height;

    QImage image(static_cast<int>(imgwidth * devicePixelRatio),
            static_cast<int>(imgheight * devicePixelRatio),
            QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(devicePixelRatio);
    image.fill(QColor(0, 0, 0, 0).rgba());

    // See comment on use of QPainter at top of file
    QPainter painter;

    painter.begin(&image);

    painter.setWorldMatrixEnabled(false);

    // draw dim outlines to increase playpos/waveform contrast
    painter.setOpacity(0.5);
    painter.setPen(m_waveformRenderer->getWaveformSignalColors()->getBgColor());
    QBrush bgFill = m_waveformRenderer->getWaveformSignalColors()->getBgColor();
    // lines next to playpos
    // Note: don't draw lines where they would overlap the triangles,
    // otherwise both translucent strokes add up to a darker tone.
    painter.drawLine(QLineF(lineX + 1.f, 4.f, lineX + 1.f, height));
    painter.drawLine(QLineF(lineX - 1.f, 4.f, lineX - 1.f, height));

    // triangle at top edge
    // Increase line/waveform contrast
    painter.setOpacity(0.8);
    {
        QPointF t0 = QPointF(lineX - 5.f, 0.f);
        QPointF t1 = QPointF(lineX + 5.f, 0.f);
        QPointF t2 = QPointF(lineX, 6.f);
        drawTriangle(&painter, bgFill, t0, t1, t2);
    }
    // draw colored play position indicators
    painter.setOpacity(1.0);
    painter.setPen(m_waveformRenderer->getWaveformSignalColors()->getPlayPosColor());
    QBrush fgFill = m_waveformRenderer->getWaveformSignalColors()->getPlayPosColor();
    // play position line
    painter.drawLine(QLineF(lineX, 0.f, lineX, height));
    // triangle at top edge
    {
        QPointF t0 = QPointF(lineX - 4.f, 0.f);
        QPointF t1 = QPointF(lineX + 4.f, 0.f);
        QPointF t2 = QPointF(lineX, 5.f);
        drawTriangle(&painter, fgFill, t0, t1, t2);
    }
    painter.end();

    m_pPlayPosMarkTexture.reset(new QOpenGLTexture(image));
    m_pPlayPosMarkTexture->setMinificationFilter(QOpenGLTexture::Linear);
    m_pPlayPosMarkTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_pPlayPosMarkTexture->setWrapMode(QOpenGLTexture::ClampToBorder);
}

void allshader::WaveformRenderMark::drawTriangle(QPainter* painter,
        const QBrush& fillColor,
        QPointF p0,
        QPointF p1,
        QPointF p2) {
    QPainterPath triangle;
    painter->setPen(Qt::NoPen);
    triangle.moveTo(p0); // ° base 1
    triangle.lineTo(p1); // > base 2
    triangle.lineTo(p2); // > peak
    triangle.lineTo(p0); // > base 1
    painter->fillPath(triangle, fillColor);
}

void allshader::WaveformRenderMark::resizeGL(int w, int h) {
    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    const float breadth =
            static_cast<float>(
                    m_waveformRenderer->getOrientation() == Qt::Vertical ? w
                                                                         : h) /
            devicePixelRatio;

    for (const auto& pMark : m_marks) {
        generateMarkImage(pMark, breadth);
    }
    generatePlayPosMarkTexture(breadth);
}

void allshader::WaveformRenderMark::onSetTrack() {
    slotCuesUpdated();

    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();
    if (!trackInfo) {
        return;
    }
    connect(trackInfo.get(),
            &Track::cuesUpdated,
            this,
            &allshader::WaveformRenderMark::slotCuesUpdated);
}

void allshader::WaveformRenderMark::slotCuesUpdated() {
    m_bCuesUpdates = true;
}

void allshader::WaveformRenderMark::checkCuesUpdated() {
    if (!m_bCuesUpdates) {
        return;
    }
    m_bCuesUpdates = false;

    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();
    if (!trackInfo) {
        return;
    }

    QList<CuePointer> loadedCues = trackInfo->getCuePoints();
    for (const CuePointer& pCue : loadedCues) {
        int hotCue = pCue->getHotCue();
        if (hotCue == Cue::kNoHotCue) {
            continue;
        }

        // Here we assume no two cues can have the same hotcue assigned,
        // because WaveformMarkSet stores one mark for each hotcue.
        WaveformMarkPointer pMark = m_marks.getHotCueMark(hotCue);
        if (pMark.isNull()) {
            continue;
        }

        QString newLabel = pCue->getLabel();
        QColor newColor = mixxx::RgbColor::toQColor(pCue->getColor());
        if (pMark->m_text.isNull() || newLabel != pMark->m_text ||
                !pMark->fillColor().isValid() ||
                newColor != pMark->fillColor()) {
            pMark->m_text = newLabel;
            int dimBrightThreshold = m_waveformRenderer->getDimBrightThreshold();
            pMark->setBaseColor(newColor, dimBrightThreshold);
            generateMarkImage(pMark, m_waveformRenderer->getBreadth());
        }
    }
}

void allshader::WaveformRenderMark::generateMarkImage(WaveformMarkPointer pMark, float breadth) {
    // Load the pixmap from file.
    // If that succeeds loading the text and stroke is skipped.
    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    if (!pMark->m_pixmapPath.isEmpty()) {
        QString path = pMark->m_pixmapPath;
        // Use devicePixelRatio to properly scale the image
        QImage image = *WImageStore::getImage(path, devicePixelRatio);
        // QImage image = QImage(path);
        //  If loading the image didn't fail, then we're done. Otherwise fall
        //  through and render a label.
        if (!image.isNull()) {
            pMark->m_image =
                    image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            // WImageStore::correctImageColors(&pMark->m_image);
            //  Set the pixel/device ratio AFTER loading the image in order to get
            //  a truly scaled source image.
            //  See https://doc.qt.io/qt-5/qimage.html#setDevicePixelRatio
            //  Also, without this some Qt-internal issue results in an offset
            //  image when calculating the center line of pixmaps in draw().
            pMark->m_image.setDevicePixelRatio(devicePixelRatio);
            pMark->m_pTexture.reset(new QOpenGLTexture(pMark->m_image));
            pMark->m_pTexture->setMinificationFilter(QOpenGLTexture::Linear);
            pMark->m_pTexture->setMagnificationFilter(QOpenGLTexture::Linear);
            pMark->m_pTexture->setWrapMode(QOpenGLTexture::ClampToBorder);
            return;
        }
    }

    {
        // See comment on use of QPainter at top of file
        QPainter painter;

        // Determine mark text.
        QString label = pMark->m_text;
        if (pMark->getHotCue() >= 0) {
            if (!label.isEmpty()) {
                label.prepend(": ");
            }
            label.prepend(QString::number(pMark->getHotCue() + 1));
            if (label.size() > kMaxCueLabelLength) {
                label = label.left(kMaxCueLabelLength - 3) + "...";
            }
        }

        // This alone would pick the OS default font, or that set by Qt5 Settings (qt5ct)
        // respectively. This would mostly not be notable since contemporary OS and distros
        // use a proven sans-serif anyway. Though, some user fonts may be lacking glyphs
        // we use for the intro/outro markers for example.
        QFont font;
        // So, let's just use Open Sans which is used by all official skins to achieve
        // a consistent skin design.
        font.setFamily("Open Sans");
        // Use a pixel size like everywhere else in Mixxx, which can be scaled well
        // in general.
        // Point sizes would work if only explicit Qt scaling QT_SCALE_FACTORS is used,
        // though as soon as other OS-based font and app scaling mechanics join the
        // party the resulting font size is hard to predict (affects all supported OS).
        font.setPixelSize(13);
        font.setWeight(75); // bold
        font.setItalic(false);

        QFontMetrics metrics(font);

        // fixed margin ...
        QRect wordRect = metrics.tightBoundingRect(label);
        wordRect.setHeight(wordRect.height() + (wordRect.height() % 2));
        wordRect.setWidth(wordRect.width() + (wordRect.width()) % 2);
        constexpr float marginX = 1.f;
        constexpr float marginY = 1.f;
        QRectF wordRectF(marginX + 1.f,
                marginY + 1.f,
                static_cast<float>(wordRect.width()),
                static_cast<float>(wordRect.height()));
        // even wordrect to have an even Image >> draw the line in the middle !

        float labelRectWidth = static_cast<float>(wordRectF.width()) + 2.f * marginX + 4.f;
        float labelRectHeight = static_cast<float>(wordRectF.height()) + 2.f * marginY + 4.f;

        QRectF labelRect(0.f, 0.f, labelRectWidth, labelRectHeight);

        float width;
        float height;

        width = 2.f * labelRectWidth + 1.f;
        height = breadth;

        pMark->m_image = QImage(
                static_cast<int>(width * devicePixelRatio),
                static_cast<int>(height * devicePixelRatio),
                QImage::Format_ARGB32_Premultiplied);
        pMark->m_image.setDevicePixelRatio(devicePixelRatio);

        Qt::Alignment markAlignH = pMark->m_align & Qt::AlignHorizontal_Mask;
        Qt::Alignment markAlignV = pMark->m_align & Qt::AlignVertical_Mask;

        if (markAlignH == Qt::AlignHCenter) {
            labelRect.moveLeft((width - labelRectWidth) / 2.f);
        } else if (markAlignH == Qt::AlignRight) {
            labelRect.moveRight(width - 1.f);
        }

        if (markAlignV == Qt::AlignVCenter) {
            labelRect.moveTop((height - labelRectHeight) / 2.f);
        } else if (markAlignV == Qt::AlignBottom) {
            labelRect.moveBottom(height - 1.f);
        }

        pMark->m_label.setAreaRect(labelRect);

        // Fill with transparent pixels
        pMark->m_image.fill(QColor(0, 0, 0, 0).rgba());

        painter.begin(&pMark->m_image);
        painter.setRenderHint(QPainter::TextAntialiasing);

        painter.setWorldMatrixEnabled(false);

        // Draw marker lines
        float middle = width / 2.f;
        pMark->m_linePosition = middle;
        if (markAlignH == Qt::AlignHCenter) {
            if (labelRect.top() > 0) {
                painter.setPen(pMark->fillColor());
                painter.drawLine(QLineF(middle, 0, middle, labelRect.top()));

                painter.setPen(pMark->borderColor());
                painter.drawLine(QLineF(middle - 1.f, 0.f, middle - 1.f, labelRect.top()));
                painter.drawLine(QLineF(middle + 1.f, 0.f, middle + 1.f, labelRect.top()));
            }

            if (labelRect.bottom() < height) {
                painter.setPen(pMark->fillColor());
                painter.drawLine(QLineF(middle, labelRect.bottom(), middle, height));

                painter.setPen(pMark->borderColor());
                painter.drawLine(QLineF(middle - 1.f,
                        labelRect.bottom(),
                        middle - 1.f,
                        height));
                painter.drawLine(QLineF(middle + 1.f,
                        labelRect.bottom(),
                        middle + 1.f,
                        height));
            }
        } else { // AlignLeft || AlignRight
            painter.setPen(pMark->fillColor());
            painter.drawLine(QLineF(middle, 0.f, middle, height));

            painter.setPen(pMark->borderColor());
            painter.drawLine(QLineF(middle - 1.f, 0, middle - 1.f, height));
            painter.drawLine(QLineF(middle + 1.f, 0, middle + 1.f, height));
        }

        // Draw the label rect
        painter.setPen(pMark->borderColor());
        painter.setBrush(QBrush(pMark->fillColor()));
        painter.drawRoundedRect(labelRect, 2.0, 2.0);

        // Draw text
        painter.setBrush(QBrush(QColor(0, 0, 0, 0)));
        painter.setFont(font);
        painter.setPen(pMark->labelColor());
        painter.drawText(labelRect, Qt::AlignCenter, label);
    }

    pMark->m_pTexture.reset(new QOpenGLTexture(pMark->m_image));
    pMark->m_pTexture->setMinificationFilter(QOpenGLTexture::Linear);
    pMark->m_pTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    pMark->m_pTexture->setWrapMode(QOpenGLTexture::ClampToBorder);
}
