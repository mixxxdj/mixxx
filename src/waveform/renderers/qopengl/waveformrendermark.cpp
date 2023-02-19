#include "waveform/renderers/qopengl/waveformrendermark.h"

#include <QDomNode>
#include <QOpenGLTexture>
#include <QPainterPath>

#include "control/controlobject.h"
#include "engine/controls/cuecontrol.h"
#include "track/track.h"
#include "util/color/color.h"
#include "util/painterscope.h"
#include "waveform/renderers/qopengl/moc_waveformrendermark.cpp"
#include "waveform/waveform.h"
#include "waveform/widgets/qopengl/waveformwidget.h"
#include "widget/wimagestore.h"
#include "widget/wskincolor.h"

namespace {
constexpr int kMaxCueLabelLength = 23;
} // namespace

qopengl::WaveformRenderMark::WaveformRenderMark(WaveformWidgetRenderer* waveformWidget)
        : WaveformRenderer(waveformWidget) {
}

qopengl::WaveformRenderMark::~WaveformRenderMark() {
    for (const auto& pMark : m_marks) {
        pMark->m_pTexture.reset();
    }
}

void qopengl::WaveformRenderMark::setup(const QDomNode& node, const SkinContext& context) {
    WaveformSignalColors signalColors = *m_waveformRenderer->getWaveformSignalColors();
    m_marks.setup(m_waveformRenderer->getGroup(), node, context, signalColors);
}

void qopengl::WaveformRenderMark::initializeGL() {
    initGradientShader();
    initTextureShader();
    for (const auto& pMark : m_marks) {
        generateMarkImage(pMark);
    }
    generatePlayPosMarkTexture();
}

void qopengl::WaveformRenderMark::initGradientShader() {
    QString vertexShaderCode = QStringLiteral(R"--(
uniform mat4 matrix;
attribute vec4 position;
attribute vec3 gradient;
varying vec3 vGradient;
void main()
{
    vGradient = gradient;
    gl_Position = matrix * position;
}
)--");

    QString fragmentShaderCode = QStringLiteral(R"--(
uniform vec4 color;
varying vec3 vGradient;
void main()
{
    gl_FragColor = vec4(color.x, color.y, color.z, color.w * max(0.0, abs((vGradient.x + vGradient.y) * 4.0 - 2.0) - 1.0));
}
)--");

    if (!m_gradientShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderCode)) {
        return;
    }

    if (!m_gradientShaderProgram.addShaderFromSourceCode(
                QOpenGLShader::Fragment, fragmentShaderCode)) {
        return;
    }

    if (!m_gradientShaderProgram.link()) {
        return;
    }

    if (!m_gradientShaderProgram.bind()) {
        return;
    }
}

void qopengl::WaveformRenderMark::initTextureShader() {
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

    if (!m_textureShaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderCode)) {
        return;
    }

    if (!m_textureShaderProgram.addShaderFromSourceCode(
                QOpenGLShader::Fragment, fragmentShaderCode)) {
        return;
    }

    if (!m_textureShaderProgram.link()) {
        return;
    }

    if (!m_textureShaderProgram.bind()) {
        return;
    }
}

void qopengl::WaveformRenderMark::drawTexture(int x, int y, QOpenGLTexture* texture) {
    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    const float texx1 = 0.f;
    const float texy1 = 0.f;
    const float texx2 = 1.f;
    const float texy2 = 1.f;

    const float posx1 = x;
    const float posx2 = x + texture->width() / devicePixelRatio;
    const float posy1 = y;
    const float posy2 = y + texture->height() / devicePixelRatio;

    const float posarray[] = {posx1, posy1, posx2, posy1, posx1, posy2, posx2, posy2};
    const float texarray[] = {texx1, texy1, texx2, texy1, texx1, texy2, texx2, texy2};

    QMatrix4x4 matrix;
    matrix.ortho(QRectF(0, 0, m_waveformRenderer->getWidth(), m_waveformRenderer->getHeight()));

    m_textureShaderProgram.bind();

    int matrixLocation = m_textureShaderProgram.uniformLocation("matrix");
    int samplerLocation = m_textureShaderProgram.uniformLocation("sampler");
    int positionLocation = m_textureShaderProgram.attributeLocation("position");
    int texcoordLocation = m_textureShaderProgram.attributeLocation("texcoor");

    m_textureShaderProgram.setUniformValue(matrixLocation, matrix);

    m_textureShaderProgram.enableAttributeArray(positionLocation);
    m_textureShaderProgram.setAttributeArray(
            positionLocation, GL_FLOAT, posarray, 2);
    m_textureShaderProgram.enableAttributeArray(texcoordLocation);
    m_textureShaderProgram.setAttributeArray(
            texcoordLocation, GL_FLOAT, texarray, 2);

    m_textureShaderProgram.setUniformValue(samplerLocation, 0);

    texture->bind();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void qopengl::WaveformRenderMark::fillRectWithGradient(
        const QRectF& rect, QColor color, Qt::Orientation orientation) {
    const float grdx1 = 0.f;
    const float grdy1 = 0.f;
    const float grdx2 = orientation == Qt::Horizontal ? 1.f : 0.f;
    const float grdy2 = orientation == Qt::Vertical ? 1.f : 0.f;

    const float posx1 = rect.x();
    const float posx2 = rect.x() + rect.width();
    const float posy1 = rect.y();
    const float posy2 = rect.y() + rect.height();

    const float posarray[] = {posx1, posy1, posx2, posy1, posx1, posy2, posx2, posy2};
    const float grdarray[] = {grdx1, grdy1, grdx2, grdy1, grdx1, grdy2, grdx2, grdy2};

    QMatrix4x4 matrix;
    matrix.ortho(QRectF(0, 0, m_waveformRenderer->getWidth(), m_waveformRenderer->getHeight()));
    m_gradientShaderProgram.bind();

    int matrixLocation = m_gradientShaderProgram.uniformLocation("matrix");
    int colorLocation = m_gradientShaderProgram.uniformLocation("color");
    int positionLocation = m_gradientShaderProgram.attributeLocation("position");
    int gradientLocation = m_gradientShaderProgram.attributeLocation("gradient");

    m_gradientShaderProgram.setUniformValue(matrixLocation, matrix);
    m_gradientShaderProgram.setUniformValue(colorLocation, color);

    m_gradientShaderProgram.enableAttributeArray(positionLocation);
    m_gradientShaderProgram.setAttributeArray(
            positionLocation, GL_FLOAT, posarray, 2);
    m_gradientShaderProgram.enableAttributeArray(gradientLocation);
    m_gradientShaderProgram.setAttributeArray(
            gradientLocation, GL_FLOAT, grdarray, 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void qopengl::WaveformRenderMark::renderGL() {
    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    QMap<WaveformMarkPointer, int> marksOnScreen;

    checkCuesUpdated();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const auto& pMark : m_marks) {
        if (!pMark->isValid()) {
            continue;
        }

        if ((pMark->hasVisible() && !pMark->isVisible()) || pMark->m_image.isNull()) {
            continue;
        }

        const double samplePosition = pMark->getSamplePosition();
        if (samplePosition != Cue::kNoPosition) {
            double currentMarkPoint =
                    m_waveformRenderer->transformSamplePositionInRendererWorld(samplePosition);
            const double sampleEndPosition = pMark->getSampleEndPosition();

            currentMarkPoint = qRound(currentMarkPoint);

            if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
                // Pixmaps are expected to have the mark stroke at the center,
                // and preferably have an odd width in order to have the stroke
                // exactly at the sample position.
                const int markHalfWidth = static_cast<int>(
                        pMark->m_image.width() / 2.0 / devicePixelRatio);
                const int drawOffset = static_cast<int>(currentMarkPoint) - markHalfWidth;

                bool visible = false;
                // Check if the current point needs to be displayed.
                if (currentMarkPoint > -markHalfWidth &&
                        currentMarkPoint < m_waveformRenderer->getWidth() +
                                        markHalfWidth) {
                    drawTexture(drawOffset, 0, pMark->m_pTexture.get());
                    visible = true;
                }

                // Check if the range needs to be displayed.
                if (samplePosition != sampleEndPosition && sampleEndPosition != Cue::kNoPosition) {
                    DEBUG_ASSERT(samplePosition < sampleEndPosition);
                    double currentMarkEndPoint =
                            m_waveformRenderer->transformSamplePositionInRendererWorld(
                                    sampleEndPosition);

                    currentMarkEndPoint = qRound(currentMarkEndPoint);

                    if (visible || currentMarkEndPoint > 0) {
                        QColor color = pMark->fillColor();
                        color.setAlphaF(0.4);

                        fillRectWithGradient(
                                QRectF(QPointF(currentMarkPoint, 0),
                                        QPointF(currentMarkEndPoint,
                                                m_waveformRenderer
                                                        ->getHeight())),
                                color,
                                Qt::Vertical);
                        visible = true;
                    }
                }

                if (visible) {
                    marksOnScreen[pMark] = drawOffset;
                }
            } else {
                const int markHalfHeight = static_cast<int>(pMark->m_image.height() / 2.0);
                const int drawOffset = static_cast<int>(currentMarkPoint) - markHalfHeight;

                bool visible = false;
                // Check if the current point needs to be displayed.
                if (currentMarkPoint > -markHalfHeight &&
                        currentMarkPoint < m_waveformRenderer->getHeight() +
                                        markHalfHeight) {
                    drawTexture(drawOffset, 0, pMark->m_pTexture.get());
                    visible = true;
                }

                // Check if the range needs to be displayed.
                if (samplePosition != sampleEndPosition && sampleEndPosition != Cue::kNoPosition) {
                    DEBUG_ASSERT(samplePosition < sampleEndPosition);
                    double currentMarkEndPoint =
                            m_waveformRenderer
                                    ->transformSamplePositionInRendererWorld(
                                            sampleEndPosition);
                    if (currentMarkEndPoint < m_waveformRenderer->getHeight()) {
                        QColor color = pMark->fillColor();
                        color.setAlphaF(0.4);
                        fillRectWithGradient(
                                QRectF(QPointF(0, currentMarkPoint),
                                        QPointF(m_waveformRenderer->getWidth(),
                                                currentMarkEndPoint)),
                                color,
                                Qt::Horizontal);
                        visible = true;
                    }
                }

                if (visible) {
                    marksOnScreen[pMark] = drawOffset;
                }
            }
        }
    }
    m_waveformRenderer->setMarkPositions(marksOnScreen);

    double currentMarkPoint =
            qRound(m_waveformRenderer->getPlayMarkerPosition() *
                    m_waveformRenderer->getWidth());
    const int markHalfWidth = static_cast<int>(
            m_pPlayPosMarkTexture->width() / 2.0 / devicePixelRatio);
    const int drawOffset = static_cast<int>(currentMarkPoint) - markHalfWidth;

    drawTexture(drawOffset, 0, m_pPlayPosMarkTexture.get());
}

void qopengl::WaveformRenderMark::generatePlayPosMarkTexture() {
    float imgwidth;
    float imgheight;

    const auto width = m_waveformRenderer->getWidth();
    const auto height = m_waveformRenderer->getHeight();
    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();

    //    const auto playMarkerPosition = m_waveformRenderer->getPlayMarkerPosition();
    const auto orientation = m_waveformRenderer->getOrientation();

    const int lineX = 5;
    const int lineY = 5;

    if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
        imgwidth = 11;
        imgheight = m_waveformRenderer->getHeight();
    } else {
        imgwidth = m_waveformRenderer->getWidth();
        imgheight = 11;
    }

    QImage image(static_cast<int>(imgwidth * devicePixelRatio),
            static_cast<int>(imgheight * devicePixelRatio),
            QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(devicePixelRatio);
    image.fill(QColor(0, 0, 0, 0).rgba());

    QPainter painter;

    painter.begin(&image);

    painter.setWorldMatrixEnabled(false);

    // draw dim outlines to increase playpos/waveform contrast
    painter.setOpacity(0.5);
    painter.setPen(m_waveformRenderer->getWaveformSignalColors()->getBgColor());
    QBrush bgFill = m_waveformRenderer->getWaveformSignalColors()->getBgColor();
    if (orientation == Qt::Horizontal) {
        // lines next to playpos
        // Note: don't draw lines where they would overlap the triangles,
        // otherwise both translucent strokes add up to a darker tone.
        painter.drawLine(lineX + 1, 4, lineX + 1, height);
        painter.drawLine(lineX - 1, 4, lineX - 1, height);

        // triangle at top edge
        // Increase line/waveform contrast
        painter.setOpacity(0.8);
        QPointF t0 = QPointF(lineX - 5, 0);
        QPointF t1 = QPointF(lineX + 5, 0);
        QPointF t2 = QPointF(lineX, 6);
        drawTriangle(&painter, bgFill, t0, t1, t2);
    } else { // vertical waveforms
        painter.drawLine(4, lineY + 1, width, lineY + 1);
        painter.drawLine(4, lineY - 1, width, lineY - 1);
        // triangle at left edge
        painter.setOpacity(0.8);
        QPointF l0 = QPointF(0, lineY - 5.01);
        QPointF l1 = QPointF(0, lineY + 4.99);
        QPointF l2 = QPointF(6, lineY);
        drawTriangle(&painter, bgFill, l0, l1, l2);
    }

    // draw colored play position indicators
    painter.setOpacity(1.0);
    painter.setPen(m_waveformRenderer->getWaveformSignalColors()->getPlayPosColor());
    QBrush fgFill = m_waveformRenderer->getWaveformSignalColors()->getPlayPosColor();
    if (orientation == Qt::Horizontal) {
        // play position line
        painter.drawLine(lineX, 0, lineX, height);
        // triangle at top edge
        QPointF t0 = QPointF(lineX - 4, 0);
        QPointF t1 = QPointF(lineX + 4, 0);
        QPointF t2 = QPointF(lineX, 5);
        drawTriangle(&painter, fgFill, t0, t1, t2);
    } else {
        // vertical waveforms
        painter.drawLine(0, lineY, width, lineY);
        // triangle at left edge
        QPointF l0 = QPointF(0, lineY - 4.01);
        QPointF l1 = QPointF(0, lineY + 4);
        QPointF l2 = QPointF(5, lineY);
        drawTriangle(&painter, fgFill, l0, l1, l2);
    }
    painter.end();

    m_pPlayPosMarkTexture.reset(new QOpenGLTexture(image));
    m_pPlayPosMarkTexture->setMinificationFilter(QOpenGLTexture::Linear);
    m_pPlayPosMarkTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_pPlayPosMarkTexture->setWrapMode(QOpenGLTexture::ClampToBorder);
}

void qopengl::WaveformRenderMark::drawTriangle(QPainter* painter,
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

void qopengl::WaveformRenderMark::resizeGL(int, int) {
    for (const auto& pMark : m_marks) {
        generateMarkImage(pMark);
    }
    generatePlayPosMarkTexture();
}

void qopengl::WaveformRenderMark::onSetTrack() {
    slotCuesUpdated();

    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();
    if (!trackInfo) {
        return;
    }
    connect(trackInfo.get(),
            &Track::cuesUpdated,
            this,
            &qopengl::WaveformRenderMark::slotCuesUpdated);
}

void qopengl::WaveformRenderMark::slotCuesUpdated() {
    m_bCuesUpdates = true;
}

void qopengl::WaveformRenderMark::checkCuesUpdated() {
    if (!m_bCuesUpdates) {
        return;
    }
    // TODO @m0dB use atomic?
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
            generateMarkImage(pMark);
        }
    }
}

void qopengl::WaveformRenderMark::generateMarkImage(WaveformMarkPointer pMark) {
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
        constexpr int marginX = 1;
        constexpr int marginY = 1;
        wordRect.moveTop(marginX + 1);
        wordRect.moveLeft(marginY + 1);
        wordRect.setHeight(wordRect.height() + (wordRect.height() % 2));
        wordRect.setWidth(wordRect.width() + (wordRect.width()) % 2);
        // even wordrect to have an even Image >> draw the line in the middle !

        int labelRectWidth = wordRect.width() + 2 * marginX + 4;
        int labelRectHeight = wordRect.height() + 2 * marginY + 4;

        QRectF labelRect(0, 0, (float)labelRectWidth, (float)labelRectHeight);

        int width;
        int height;

        if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
            width = 2 * labelRectWidth + 1;
            height = m_waveformRenderer->getHeight();
        } else {
            width = m_waveformRenderer->getWidth();
            height = 2 * labelRectHeight + 1;
        }

        pMark->m_image = QImage(
                static_cast<int>(width * devicePixelRatio),
                static_cast<int>(height * devicePixelRatio),
                QImage::Format_ARGB32_Premultiplied);
        pMark->m_image.setDevicePixelRatio(devicePixelRatio);

        Qt::Alignment markAlignH = pMark->m_align & Qt::AlignHorizontal_Mask;
        Qt::Alignment markAlignV = pMark->m_align & Qt::AlignVertical_Mask;

        if (markAlignH == Qt::AlignHCenter) {
            labelRect.moveLeft((width - labelRectWidth) / 2);
        } else if (markAlignH == Qt::AlignRight) {
            labelRect.moveRight(width - 1);
        }

        if (markAlignV == Qt::AlignVCenter) {
            labelRect.moveTop((height - labelRectHeight) / 2);
        } else if (markAlignV == Qt::AlignBottom) {
            labelRect.moveBottom(height - 1);
        }

        pMark->m_label.setAreaRect(labelRect);

        // Fill with transparent pixels
        pMark->m_image.fill(QColor(0, 0, 0, 0).rgba());

        painter.begin(&pMark->m_image);
        painter.setRenderHint(QPainter::TextAntialiasing);

        painter.setWorldMatrixEnabled(false);

        // Draw marker lines
        if (m_waveformRenderer->getOrientation() == Qt::Horizontal) {
            int middle = width / 2;
            pMark->m_linePosition = middle;
            if (markAlignH == Qt::AlignHCenter) {
                if (labelRect.top() > 0) {
                    painter.setPen(pMark->fillColor());
                    painter.drawLine(QLineF(middle, 0, middle, labelRect.top()));

                    painter.setPen(pMark->borderColor());
                    painter.drawLine(QLineF(middle - 1, 0, middle - 1, labelRect.top()));
                    painter.drawLine(QLineF(middle + 1, 0, middle + 1, labelRect.top()));
                }

                if (labelRect.bottom() < height) {
                    painter.setPen(pMark->fillColor());
                    painter.drawLine(QLineF(middle, labelRect.bottom(), middle, height));

                    painter.setPen(pMark->borderColor());
                    painter.drawLine(QLineF(middle - 1, labelRect.bottom(), middle - 1, height));
                    painter.drawLine(QLineF(middle + 1, labelRect.bottom(), middle + 1, height));
                }
            } else { // AlignLeft || AlignRight
                painter.setPen(pMark->fillColor());
                painter.drawLine(middle, 0, middle, height);

                painter.setPen(pMark->borderColor());
                painter.drawLine(middle - 1, 0, middle - 1, height);
                painter.drawLine(middle + 1, 0, middle + 1, height);
            }
        } else { // Vertical
            int middle = height / 2;
            pMark->m_linePosition = middle;
            if (markAlignV == Qt::AlignVCenter) {
                if (labelRect.left() > 0) {
                    painter.setPen(pMark->fillColor());
                    painter.drawLine(QLineF(0, middle, labelRect.left(), middle));

                    painter.setPen(pMark->borderColor());
                    painter.drawLine(QLineF(0, middle - 1, labelRect.left(), middle - 1));
                    painter.drawLine(QLineF(0, middle + 1, labelRect.left(), middle + 1));
                }

                if (labelRect.right() < width) {
                    painter.setPen(pMark->fillColor());
                    painter.drawLine(QLineF(labelRect.right(), middle, width, middle));

                    painter.setPen(pMark->borderColor());
                    painter.drawLine(QLineF(labelRect.right(), middle - 1, width, middle - 1));
                    painter.drawLine(QLineF(labelRect.right(), middle + 1, width, middle + 1));
                }
            } else { // AlignTop || AlignBottom
                painter.setPen(pMark->fillColor());
                painter.drawLine(0, middle, width, middle);

                painter.setPen(pMark->borderColor());
                painter.drawLine(0, middle - 1, width, middle - 1);
                painter.drawLine(0, middle + 1, width, middle + 1);
            }
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
