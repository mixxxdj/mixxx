#include "waveform/renderers/allshader/waveformrendermark.h"

#include <QDomNode>
#include <QOpenGLTexture>
#include <QPainterPath>

#include "track/track.h"
#include "util/color/color.h"
#include "util/colorcomponents.h"
#include "util/texture.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/renderers/allshader/moc_waveformrendermark.cpp"
#include "waveform/renderers/allshader/rgbadata.h"
#include "waveform/renderers/allshader/vertexdata.h"
#include "waveform/renderers/waveformsignalcolors.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wimagestore.h"

// On the use of QPainter:
//
// The renderers in this folder are optimized to use GLSL shaders and refrain
// from using QPainter on the QOpenGLWindow, which causes degredated performance.
//
// This renderer does use QPainter (indirectly, in WaveformMark::generateImage), but
// only to draw on a QImage. This is only done once when needed and the images are
// then used as textures to be drawn with a GLSL shader.

class TextureGraphics : public WaveformMark::Graphics {
    std::unique_ptr<QOpenGLTexture> m_pTexture;

  public:
    TextureGraphics(std::unique_ptr<QOpenGLTexture>&& pTexture)
            : m_pTexture{std::move(pTexture)} {
    }
    QOpenGLTexture* texture() const {
        return m_pTexture.get();
    }
};

allshader::WaveformRenderMark::WaveformRenderMark(WaveformWidgetRenderer* waveformWidget)
        : WaveformRenderer(waveformWidget),
          m_bCuesUpdates(false) {
}

void allshader::WaveformRenderMark::setup(const QDomNode& node, const SkinContext& context) {
    WaveformSignalColors signalColors = *m_waveformRenderer->getWaveformSignalColors();
    m_marks.setup(m_waveformRenderer->getGroup(), node, context, signalColors);
}

void allshader::WaveformRenderMark::initializeGL() {
    WaveformRenderer::initializeGL();
    m_rgbaShader.init();
    m_textureShader.init();

    for (const auto& pMark : std::as_const(m_marks)) {
        generateMarkImage(pMark);
    }
    generatePlayPosMarkTexture();
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
    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    QList<WaveformWidgetRenderer::WaveformMarkOnScreen> marksOnScreen;

    checkCuesUpdated();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const auto& pMark : std::as_const(m_marks)) {
        if (!pMark->m_pGraphics || pMark->m_pGraphics->m_obsolete) {
            generateMarkImage(pMark);
        }

        QOpenGLTexture* pTexture =
                static_cast<TextureGraphics*>(pMark->m_pGraphics.get())
                        ->texture();

        const double samplePosition = pMark->getSamplePosition();
        if (samplePosition != Cue::kNoPosition) {
            const float currentMarkPoint = std::floor(static_cast<float>(
                    m_waveformRenderer->transformSamplePositionInRendererWorld(
                            samplePosition)));
            const double sampleEndPosition = pMark->getSampleEndPosition();

            // Pixmaps are expected to have the mark stroke at the center,
            // and preferably have an odd width in order to have the stroke
            // exactly at the sample position.
            const float markHalfWidth = pTexture->width() / devicePixelRatio / 2.f;
            const float drawOffset = currentMarkPoint - markHalfWidth;

            bool visible = false;
            // Check if the current point needs to be displayed.
            if (currentMarkPoint > -markHalfWidth &&
                    currentMarkPoint < m_waveformRenderer->getLength() +
                                    markHalfWidth) {
                drawTexture(drawOffset, 0, pTexture);
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
                    color.setAlphaF(0.4f);

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
                marksOnScreen.append(
                        WaveformWidgetRenderer::WaveformMarkOnScreen{
                                pMark, static_cast<int>(drawOffset)});
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

// Generate the texture used to draw the play position marker.
// Note that in the legacy waveform widgets this is drawn directly
// in the WaveformWidgetRenderer itself. Doing it here is cleaner.
void allshader::WaveformRenderMark::generatePlayPosMarkTexture() {
    float imgwidth;
    float imgheight;

    const float height = m_waveformRenderer->getBreadth();
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
    painter.drawLine(QLineF(lineX + 1.f, 4.f, lineX + 1.f, height));
    painter.drawLine(QLineF(lineX - 1.f, 4.f, lineX - 1.f, height));

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
    painter.drawLine(QLineF(lineX, 0.f, lineX, height));
    // triangle at top edge
    {
        QPointF baseL = QPointF(lineX - 4.f, 0.f);
        QPointF baseR = QPointF(lineX + 4.f, 0.f);
        QPointF tip = QPointF(lineX, 4.f);
        drawTriangle(&painter, fgColor, baseL, baseR, tip);
    }
    painter.end();

    m_pPlayPosMarkTexture = createTexture(image);
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
    for (const auto& pMark : std::as_const(m_marks)) {
        generateMarkImage(pMark);
    }
    generatePlayPosMarkTexture();
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
        const int hotCue = pCue->getHotCue();
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
            const int dimBrightThreshold = m_waveformRenderer->getDimBrightThreshold();
            pMark->setBaseColor(newColor, dimBrightThreshold);
            generateMarkImage(pMark);
        }
    }

    m_marks.update();
}

void allshader::WaveformRenderMark::generateMarkImage(WaveformMarkPointer pMark) {
    pMark->m_pGraphics = std::make_unique<TextureGraphics>(
            createTexture(pMark->generateImage(m_waveformRenderer->getBreadth(),
                    m_waveformRenderer->getDevicePixelRatio())));
}
