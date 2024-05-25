#include "waveform/renderers/allshader/waveformrendererpreroll.h"

#include <QDomNode>
#include <QOpenGLTexture>
#include <QPainterPath>
#include <array>

#include "skin/legacy/skincontext.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"

namespace {
QImage drawPrerollImage(float markerLength,
        float markerBreadth,
        float devicePixelRatio,
        QColor color) {
    const int imagePixelW = static_cast<int>(markerLength * devicePixelRatio + 0.5f);
    const int imagePixelH = static_cast<int>(markerBreadth * devicePixelRatio + 0.5f);
    const float imageW = static_cast<float>(imagePixelW) / devicePixelRatio;
    const float imageH = static_cast<float>(imagePixelH) / devicePixelRatio;

    QImage image(imagePixelW, imagePixelH, QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(devicePixelRatio);

    const float penWidth = 1.5f;
    const float offset = penWidth / 2.f;

    image.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&image);

    painter.setWorldMatrixEnabled(false);

    QPen pen(color);
    pen.setWidthF(penWidth);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.setRenderHints(QPainter::Antialiasing);
    // Draw base the right, tip to the left
    QPointF p0{imageW - offset, offset};
    QPointF p1{imageW - offset, imageH - offset};
    QPointF p2{offset, imageH / 2.f};
    QPainterPath path;
    path.moveTo(p2);
    path.lineTo(p1);
    path.lineTo(p0);
    path.closeSubpath();
    QColor fillColor = color;
    fillColor.setAlphaF(0.5f);
    painter.fillPath(path, QBrush(fillColor));

    painter.drawPath(path);
    painter.end();

    return image;
}
} // anonymous namespace

namespace allshader {

WaveformRendererPreroll::WaveformRendererPreroll(
        WaveformWidgetRenderer* waveformWidget,
        ::WaveformRendererAbstract::PositionSource type)
        : WaveformRenderer(waveformWidget),
          m_isSlipRenderer(type == ::WaveformRendererAbstract::Slip) {
}

WaveformRendererPreroll::~WaveformRendererPreroll() = default;

void WaveformRendererPreroll::setup(
        const QDomNode& node, const SkinContext& context) {
    m_color = QColor(context.selectString(node, "SignalColor"));
    m_color = WSkinColor::getCorrectColor(m_color);
}

void WaveformRendererPreroll::initializeGL() {
    WaveformRenderer::initializeGL();
    m_shader.init();
}

void WaveformRendererPreroll::paintGL() {
    const TrackPointer track = m_waveformRenderer->getTrackInfo();
    if (!track || (m_isSlipRenderer && !m_waveformRenderer->isSlipActive())) {
        return;
    }

    auto positionType = m_isSlipRenderer ? ::WaveformRendererAbstract::Slip
                                         : ::WaveformRendererAbstract::Play;

    const double firstDisplayedPosition =
            m_waveformRenderer->getFirstDisplayedPosition(positionType);
    const double lastDisplayedPosition = m_waveformRenderer->getLastDisplayedPosition(positionType);

    // Check if the pre- or post-roll is on screen. If so, draw little triangles
    // to indicate the respective zones.
    const bool preRollVisible = firstDisplayedPosition < 0;
    const bool postRollVisible = lastDisplayedPosition > 1;

    if (!(preRollVisible || postRollVisible)) {
        return;
    }

    const double playMarkerPosition = m_waveformRenderer->getPlayMarkerPosition();
    const double vSamplesPerPixel = m_waveformRenderer->getVisualSamplePerPixel();
    const double numberOfVSamples = m_waveformRenderer->getLength() * vSamplesPerPixel;

    const int currentVSamplePosition = m_waveformRenderer->getPlayPosVSample(positionType);
    const int totalVSamples = m_waveformRenderer->getTotalVSample();

    const float markerBreadth = m_waveformRenderer->getBreadth() * 0.4f;

    const float halfBreadth = m_waveformRenderer->getBreadth() * 0.5f;
    const float halfMarkerBreadth = markerBreadth * 0.5f;

    const float markerLength = 40.f / static_cast<float>(vSamplesPerPixel);

    // A series of markers will be drawn (by repeating the texture in a pattern)
    // from the left of the screen up until start (preroll) and from the right
    // of the screen up until the end (postroll) of the track respectively.

    const float epsilon = 0.5f;
    if (std::abs(m_markerLength - markerLength) > epsilon ||
            std::abs(m_markerBreadth - markerBreadth) > epsilon) {
        // Regenerate the texture with the preroll marker (a triangle) if the size
        // has changed size last time.
        m_markerLength = markerLength;
        m_markerBreadth = markerBreadth;
        m_texture.setData(drawPrerollImage(m_markerLength,
                m_markerBreadth,
                m_waveformRenderer->getDevicePixelRatio(),
                m_color));
    }

    if (!m_texture.isStorageAllocated()) {
        return;
    }

    const int matrixLocation = m_shader.matrixLocation();
    const int textureLocation = m_shader.textureLocation();
    const int positionLocation = m_shader.positionLocation();
    const int texcoordLocation = m_shader.texcoordLocation();

    // Set up the shader
    m_shader.bind();

    m_shader.enableAttributeArray(positionLocation);
    m_shader.enableAttributeArray(texcoordLocation);

    const QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, false);

    m_shader.setUniformValue(matrixLocation, matrix);
    m_shader.setUniformValue(textureLocation, 0);

    m_texture.bind();

    const float end = m_waveformRenderer->getLength();

    if (preRollVisible) {
        // VSample position of the right-most triangle's tip
        const double triangleTipVSamplePosition =
                playMarkerPosition * numberOfVSamples -
                currentVSamplePosition;
        // In pixels
        float x = static_cast<float>(triangleTipVSamplePosition / vSamplesPerPixel);
        const float limit = end + markerLength;
        if (x >= limit) {
            // Don't draw invisible triangles beyond the right side of the display
            x -= std::ceil((x - limit) / markerLength) * markerLength;
        }

        drawPattern(x,
                halfBreadth - halfMarkerBreadth,
                0.f,
                m_isSlipRenderer ? halfBreadth : halfBreadth + halfMarkerBreadth,
                x / markerLength);
    }

    if (postRollVisible) {
        const int remainingVSamples = totalVSamples - currentVSamplePosition;
        // Sample position of the left-most triangle's tip
        const double triangleTipVSamplePosition =
                playMarkerPosition * numberOfVSamples +
                remainingVSamples;
        // In pixels
        float x = static_cast<float>(triangleTipVSamplePosition / vSamplesPerPixel);
        const float limit = -markerLength;
        if (x <= limit) {
            // Don't draw invisible triangles before the left side of the display
            x += std::ceil((limit - x) / markerLength) * markerLength;
        }

        drawPattern(x,
                halfBreadth - halfMarkerBreadth,
                end,
                m_isSlipRenderer ? halfBreadth : halfBreadth + halfMarkerBreadth,
                (end - x) / markerLength);
    }

    m_texture.release();

    m_shader.disableAttributeArray(positionLocation);
    m_shader.disableAttributeArray(texcoordLocation);
    m_shader.release();
}

void WaveformRendererPreroll::drawPattern(
        float x1, float y1, float x2, float y2, float repetitions) {
    // Draw a large rectangle with a repeating pattern of the texture
    const int repetitionsLocation = m_shader.repetitionsLocation();
    const int positionLocation = m_shader.positionLocation();
    const int texcoordLocation = m_shader.texcoordLocation();

    const std::array<float, 8> positionArray = {x1, y1, x2, y1, x1, y2, x2, y2};
    const std::array<float, 8> texcoordArray = {0.f,
            0.f,
            1.f,
            0.f,
            0.f,
            m_isSlipRenderer ? 0.5f : 1.f,
            1.f,
            m_isSlipRenderer ? 0.5f : 1.f};
    m_shader.setUniformValue(repetitionsLocation, QVector2D(repetitions, 1.0));

    m_shader.setAttributeArray(
            positionLocation, GL_FLOAT, positionArray.data(), 2);
    m_shader.setAttributeArray(
            texcoordLocation, GL_FLOAT, texcoordArray.data(), 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

} // namespace allshader
