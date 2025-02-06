#include "waveform/renderers/allshader/waveformrendermark.h"

#include <QOpenGLTexture>
#include <QPainterPath>

#include "track/track.h"
#include "util/colorcomponents.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/renderers/allshader/rgbadata.h"
#include "waveform/renderers/allshader/vertexdata.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveformwidgetfactory.h"

// On the use of QPainter:
//
// The renderers in this folder are optimized to use GLSL shaders and refrain
// from using QPainter on the QOpenGLWindow, which causes degredated performance.
//
// This renderer does use QPainter (indirectly, in WaveformMark::generateImage), but
// only to draw on a QImage. This is only done once when needed and the images are
// then used as textures to be drawn with a GLSL shader.

class TextureGraphics : public WaveformMark::Graphics {
  public:
    TextureGraphics(const QImage& image) {
        m_texture.setData(image);
    }
    QOpenGLTexture* texture() {
        return &m_texture;
    }

  private:
    OpenGLTexture2D m_texture;
};

// Both allshader::WaveformRenderMark and the non-GL ::WaveformRenderMark derive
// from WaveformRenderMarkBase. The base-class takes care of updating the marks
// when needed and flagging them when their image needs to be updated (resizing,
// cue changes, position changes)
//
// While in the case of ::WaveformRenderMark those images can be updated immediately,
// in the case of allshader::WaveformRenderMark we need to do that when we have an
// openGL context, as we create new textures.
//
// The boolean argument for the WaveformRenderMarkBase constructor indicates
// that updateMarkImages should not be called immediately.

namespace {
QString timeSecToString(double timeSec) {
    int hundredths = std::lround(timeSec * 100.0);
    int seconds = hundredths / 100;
    hundredths -= seconds * 100;
    int minutes = seconds / 60;
    seconds -= minutes * 60;

    return QString::asprintf("%d:%02d.%02d", minutes, seconds, hundredths);
}

} // namespace

allshader::WaveformRenderMark::WaveformRenderMark(
        WaveformWidgetRenderer* waveformWidget,
        ::WaveformRendererAbstract::PositionSource type)
        : ::WaveformRenderMarkBase(waveformWidget, false),
          m_beatsUntilMark(0),
          m_timeUntilMark(0.0),
          m_pTimeRemainingControl(nullptr),
          m_isSlipRenderer(type == ::WaveformRendererAbstract::Slip) {
}

bool allshader::WaveformRenderMark::init() {
    m_pTimeRemainingControl = std::make_unique<ControlProxy>(
            m_waveformRenderer->getGroup(), "time_remaining");
    return true;
}

void allshader::WaveformRenderMark::initializeGL() {
    allshader::WaveformRendererAbstract::initializeGL();
    m_digitsRenderer.init();
    m_rgbaShader.init();
    m_textureShader.init();

    // Will create textures so requires OpenGL context
    updateMarkImages();
    updatePlayPosMarkTexture();
    const auto untilMarkTextPointSize =
            WaveformWidgetFactory::instance()->getUntilMarkTextPointSize();
    const auto untilMarkTextHeightLimit =
            WaveformWidgetFactory::instance()
                    ->getUntilMarkTextHeightLimit(); // proportion of waveform
                                                     // height
    const auto untilMarkMaxHeightForText = getMaxHeightForText(untilMarkTextHeightLimit);

    m_digitsRenderer.updateTexture(untilMarkTextPointSize,
            untilMarkMaxHeightForText,
            m_waveformRenderer->getDevicePixelRatio());
}

void allshader::WaveformRenderMark::drawTexture(
        const QMatrix4x4& matrix, float x, float y, QOpenGLTexture* pTexture) {
    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    const float texx1 = 0.f;
    const float texy1 = 0.f;
    const float texx2 = 1.f;
    const float texy2 = 1.f;

    const float posx1 = x;
    const float posx2 = x + static_cast<float>(pTexture->width() / devicePixelRatio);
    const float posy1 = y;
    const float posy2 = y + static_cast<float>(pTexture->height() / devicePixelRatio);

    const float posarray[] = {posx1, posy1, posx2, posy1, posx1, posy2, posx2, posy2};
    const float texarray[] = {texx1, texy1, texx2, texy1, texx1, texy2, texx2, texy2};

    m_textureShader.bind();

    const int matrixLocation = m_textureShader.uniformLocation("matrix");
    const int textureLocation = m_textureShader.uniformLocation("texture");
    const int positionLocation = m_textureShader.attributeLocation("position");
    const int texcoordLocation = m_textureShader.attributeLocation("texcoord");

    m_textureShader.setUniformValue(matrixLocation, matrix);

    m_textureShader.enableAttributeArray(positionLocation);
    m_textureShader.setAttributeArray(
            positionLocation, GL_FLOAT, posarray, 2);
    m_textureShader.enableAttributeArray(texcoordLocation);
    m_textureShader.setAttributeArray(
            texcoordLocation, GL_FLOAT, texarray, 2);

    m_textureShader.setUniformValue(textureLocation, 0);

    pTexture->bind();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    pTexture->release();

    m_textureShader.disableAttributeArray(positionLocation);
    m_textureShader.disableAttributeArray(texcoordLocation);
    m_textureShader.release();
}

void allshader::WaveformRenderMark::drawMark(
        const QMatrix4x4& matrix, const QRectF& rect, QColor color) {
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

    m_rgbaShader.bind();

    const int matrixLocation = m_rgbaShader.matrixLocation();
    const int positionLocation = m_rgbaShader.positionLocation();
    const int colorLocation = m_rgbaShader.colorLocation();

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
    if (m_isSlipRenderer && !m_waveformRenderer->isSlipActive()) {
        return;
    }

    auto positionType = m_isSlipRenderer ? ::WaveformRendererAbstract::Slip
                                         : ::WaveformRendererAbstract::Play;
    bool slipActive = m_waveformRenderer->isSlipActive();

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    QList<WaveformWidgetRenderer::WaveformMarkOnScreen> marksOnScreen;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const auto& pMark : std::as_const(m_marks)) {
        pMark->setBreadth(slipActive ? m_waveformRenderer->getBreadth() / 2
                                     : m_waveformRenderer->getBreadth());
    }
    // Will create textures so requires OpenGL context
    updateMarkImages();

    QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, false);

    const double playPosition = m_waveformRenderer->getTruePosSample();
    double nextMarkPosition = std::numeric_limits<double>::max();

    for (const auto& pMark : std::as_const(m_marks)) {
        if (!pMark->isValid()) {
            continue;
        }

        const double samplePosition = pMark->getSamplePosition();

        if (samplePosition == Cue::kNoPosition) {
            continue;
        }

        QOpenGLTexture* pTexture =
                static_cast<TextureGraphics*>(pMark->m_pGraphics.get())
                        ->texture();

        if (!pTexture->isCreated()) {
            // This happens if the height is zero
            continue;
        }

        const float currentMarkPoint =
                std::round(
                        static_cast<float>(
                                m_waveformRenderer
                                        ->transformSamplePositionInRendererWorld(
                                                samplePosition)) *
                        devicePixelRatio) /
                devicePixelRatio;
        if (pMark->isShowUntilNext() &&
                samplePosition >= playPosition + 1.0 &&
                samplePosition < nextMarkPosition) {
            nextMarkPosition = samplePosition;
        }
        const double sampleEndPosition = pMark->getSampleEndPosition();

        // Pixmaps are expected to have the mark stroke at the center,
        // and preferably have an odd width in order to have the stroke
        // exactly at the sample position.
        const float markHalfWidth = pTexture->width() / devicePixelRatio / 2.f;
        const float drawOffset = currentMarkPoint - markHalfWidth;

        bool visible = false;
        // Check if the current point needs to be displayed.
        if (drawOffset > -markHalfWidth &&
                drawOffset < m_waveformRenderer->getLength() +
                                markHalfWidth) {
            drawTexture(matrix,
                    drawOffset,
                    !m_isSlipRenderer && slipActive
                            ? m_waveformRenderer->getBreadth() / 2
                            : 0,
                    pTexture);
            visible = true;
        }

        // Check if the range needs to be displayed.
        if (samplePosition != sampleEndPosition && sampleEndPosition != Cue::kNoPosition) {
            DEBUG_ASSERT(samplePosition < sampleEndPosition);
            const float currentMarkEndPoint = static_cast<
                    float>(
                    m_waveformRenderer
                            ->transformSamplePositionInRendererWorld(
                                    sampleEndPosition, positionType));

            if (visible || currentMarkEndPoint > 0) {
                QColor color = pMark->fillColor();
                color.setAlphaF(0.4f);

                drawMark(matrix,
                        QRectF(QPointF(currentMarkPoint, 0),
                                QPointF(currentMarkEndPoint,
                                        m_waveformRenderer
                                                ->getBreadth())),
                        color);
                visible = true;
            }
        }

        if (visible) {
            marksOnScreen.append(
                    WaveformWidgetRenderer::WaveformMarkOnScreen{
                            pMark, static_cast<int>(drawOffset)});
        }
    }
    m_waveformRenderer->setMarkPositions(marksOnScreen);

    const float currentMarkPoint =
            std::round(static_cast<float>(
                               m_waveformRenderer->getPlayMarkerPosition() *
                               m_waveformRenderer->getLength()) *
                    devicePixelRatio) /
            devicePixelRatio;

    if (m_playPosMarkTexture.isStorageAllocated()) {
        const float markHalfWidth = m_playPosMarkTexture.width() / devicePixelRatio / 2.f;
        const float drawOffset = currentMarkPoint - markHalfWidth;

        drawTexture(matrix, drawOffset, 0.f, &m_playPosMarkTexture);
    }

    if (WaveformWidgetFactory::instance()->getUntilMarkShowBeats() ||
            WaveformWidgetFactory::instance()->getUntilMarkShowTime()) {
        updateUntilMark(playPosition, nextMarkPosition);
        drawUntilMark(matrix, currentMarkPoint + 20);
    }
}

void allshader::WaveformRenderMark::drawUntilMark(const QMatrix4x4& matrix, float x) {
    const bool untilMarkShowBeats = WaveformWidgetFactory::instance()->getUntilMarkShowBeats();
    const bool untilMarkShowTime = WaveformWidgetFactory::instance()->getUntilMarkShowTime();
    const auto untilMarkAlign = WaveformWidgetFactory::instance()->getUntilMarkAlign();

    const auto untilMarkTextPointSize =
            WaveformWidgetFactory::instance()->getUntilMarkTextPointSize();
    const auto untilMarkTextHeightLimit =
            WaveformWidgetFactory::instance()
                    ->getUntilMarkTextHeightLimit(); // proportion of waveform
                                                     // height
    const auto untilMarkMaxHeightForText = getMaxHeightForText(untilMarkTextHeightLimit);

    m_digitsRenderer.updateTexture(untilMarkTextPointSize,
            untilMarkMaxHeightForText,
            m_waveformRenderer->getDevicePixelRatio());

    if (m_timeUntilMark == 0.0) {
        return;
    }
    const float ch = m_digitsRenderer.height();

    float y = untilMarkAlign == Qt::AlignTop ? 0.f
            : untilMarkAlign == Qt::AlignBottom
            ? m_waveformRenderer->getBreadth() - ch
            : m_waveformRenderer->getBreadth() / 2.f;

    bool multiLine = untilMarkShowBeats && untilMarkShowTime &&
            ch * 2.f < untilMarkMaxHeightForText;

    if (multiLine) {
        if (untilMarkAlign != Qt::AlignTop) {
            y -= ch;
        }
    } else {
        if (untilMarkAlign != Qt::AlignTop && untilMarkAlign != Qt::AlignBottom) {
            // center
            y -= ch / 2.f;
        }
    }

    if (untilMarkShowBeats) {
        const auto w = m_digitsRenderer.draw(matrix,
                x,
                y,
                QString::number(m_beatsUntilMark));
        if (multiLine) {
            y += ch;
        } else {
            x += w + ch * 0.75f;
        }
    }

    if (untilMarkShowTime) {
        m_digitsRenderer.draw(matrix,
                x,
                y,
                timeSecToString(m_timeUntilMark));
    }
}

// Generate the texture used to draw the play position marker.
// Note that in the legacy waveform widgets this is drawn directly
// in the WaveformWidgetRenderer itself. Doing it here is cleaner.
void allshader::WaveformRenderMark::updatePlayPosMarkTexture() {
    const float imgHeight = m_waveformRenderer->getBreadth();
    const float imgWidth = 11.f;

    if (imgHeight == 0.0f) {
        return;
    }

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    const float lineX = 5.5f;

    QImage image(static_cast<int>(imgWidth * devicePixelRatio),
            static_cast<int>(imgHeight * devicePixelRatio),
            QImage::Format_ARGB32_Premultiplied);
    VERIFY_OR_DEBUG_ASSERT(!image.isNull()) {
        return;
    }
    image.setDevicePixelRatio(devicePixelRatio);
    image.fill(QColor(0, 0, 0, 0).rgba());

    // See comment on use of QPainter at top of file
    QPainter painter;

    painter.begin(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setWorldMatrixEnabled(false);

    const QColor fgColor{m_waveformRenderer->getWaveformSignalColors()->getPlayPosColor()};
    const QColor bgColor{m_waveformRenderer->getWaveformSignalColors()->getBgColor()};

    // draw dim outlines to increase playpos/waveform contrast
    painter.setPen(bgColor);
    painter.setOpacity(0.5);
    // lines next to playpos
    // Note: don't draw lines where they would overlap the triangles,
    // otherwise both translucent strokes add up to a darker tone.
    painter.drawLine(QLineF(lineX + 1.f, 4.f, lineX + 1.f, imgHeight));
    painter.drawLine(QLineF(lineX - 1.f, 4.f, lineX - 1.f, imgHeight));

    // triangle at top edge
    // Increase line/waveform contrast
    painter.setOpacity(0.8);
    {
        QPointF baseL = QPointF(lineX - 5.f, 0.f);
        QPointF baseR = QPointF(lineX + 5.f, 0.f);
        QPointF tip = QPointF(lineX, 5.f);
        drawTriangle(&painter, bgColor, baseL, baseR, tip);
    }
    // draw colored play position indicators
    painter.setPen(fgColor);
    painter.setOpacity(1.0);
    // play position line
    painter.drawLine(QLineF(lineX, 0.f, lineX, imgHeight));
    // triangle at top edge
    {
        QPointF baseL = QPointF(lineX - 4.f, 0.f);
        QPointF baseR = QPointF(lineX + 4.f, 0.f);
        QPointF tip = QPointF(lineX, 4.f);
        drawTriangle(&painter, fgColor, baseL, baseR, tip);
    }
    painter.end();

    m_playPosMarkTexture.setData(image);
}

void allshader::WaveformRenderMark::drawTriangle(QPainter* painter,
        const QBrush& fillColor,
        QPointF baseL,
        QPointF baseR,
        QPointF tip) {
    QPainterPath triangle;
    painter->setPen(Qt::NoPen);
    triangle.moveTo(baseL);
    triangle.lineTo(tip);
    triangle.lineTo(baseR);
    triangle.closeSubpath();
    painter->fillPath(triangle, fillColor);
}

void allshader::WaveformRenderMark::resizeGL(int, int) {
    // Will create textures so requires OpenGL context
    updateMarkImages();
    updatePlayPosMarkTexture();
}

void allshader::WaveformRenderMark::updateMarkImage(WaveformMarkPointer pMark) {
    pMark->m_pGraphics = std::make_unique<TextureGraphics>(
            pMark->generateImage(m_waveformRenderer->getDevicePixelRatio()));
}

void allshader::WaveformRenderMark::updateUntilMark(
        double playPosition, double nextMarkPosition) {
    m_beatsUntilMark = 0;
    m_timeUntilMark = 0.0;
    if (nextMarkPosition == std::numeric_limits<double>::max()) {
        return;
    }

    TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();

    if (!trackInfo) {
        return;
    }

    const double endPosition = m_waveformRenderer->getTrackSamples();
    const double remainingTime = m_pTimeRemainingControl->get();

    mixxx::BeatsPointer trackBeats = trackInfo->getBeats();
    if (!trackBeats) {
        return;
    }

    auto itA = trackBeats->iteratorFrom(
            mixxx::audio::FramePos::fromEngineSamplePos(playPosition));
    auto itB = trackBeats->iteratorFrom(
            mixxx::audio::FramePos::fromEngineSamplePos(nextMarkPosition));

    // itB is the beat at or after the nextMarkPosition.
    if (itB->toEngineSamplePos() > nextMarkPosition) {
        // if itB is after nextMarkPosition, the previous beat might be closer
        // and it the one we are interested in
        if (nextMarkPosition - (itB - 1)->toEngineSamplePos() <
                itB->toEngineSamplePos() - nextMarkPosition) {
            itB--;
        }
    }

    if (std::abs(itA->toEngineSamplePos() - playPosition) < 1) {
        m_currentBeatPosition = itA->toEngineSamplePos();
        m_beatsUntilMark = std::distance(itA, itB);
        itA++;
        m_nextBeatPosition = itA->toEngineSamplePos();
    } else {
        m_nextBeatPosition = itA->toEngineSamplePos();
        itA--;
        m_currentBeatPosition = itA->toEngineSamplePos();
        m_beatsUntilMark = std::distance(itA, itB);
    }
    // As endPosition - playPosition corresponds with remainingTime,
    // we calculate the proportional part of nextMarkPosition - playPosition
    m_timeUntilMark = std::max(0.0,
            remainingTime * (nextMarkPosition - playPosition) /
                    (endPosition - playPosition));
}

float allshader::WaveformRenderMark::getMaxHeightForText(float proportion) const {
    return std::roundf(m_waveformRenderer->getBreadth() * proportion);
}
