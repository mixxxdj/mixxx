#include "waveform/renderers/allshader/waveformrendererpreroll.h"

#include <QDomNode>
#include <QPainterPath>
#include <array>

#include "rendergraph/geometry.h"
#include "rendergraph/material/patternmaterial.h"
#include "rendergraph/vertexupdaters/texturedvertexupdater.h"
#include "skin/legacy/skincontext.h"
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
    fillColor.setAlphaF(0.25f);
    painter.fillPath(path, QBrush(fillColor));

    painter.drawPath(path);
    painter.end();

    return image;
}
} // anonymous namespace

using namespace rendergraph;

namespace allshader {

WaveformRendererPreroll::WaveformRendererPreroll(
        WaveformWidgetRenderer* waveformWidget,
        ::WaveformRendererAbstract::PositionSource type)
        : ::WaveformRendererAbstract(waveformWidget),
          m_isSlipRenderer(type == ::WaveformRendererAbstract::Slip) {
    setGeometry(std::make_unique<Geometry>(PatternMaterial::attributes(), 0));
    setMaterial(std::make_unique<PatternMaterial>());
    setUsePreprocess(true);
    geometry().setDrawingMode(Geometry::DrawingMode::Triangles);
}

WaveformRendererPreroll::~WaveformRendererPreroll() = default;

void WaveformRendererPreroll::setup(
        const QDomNode& node, const SkinContext& skinContext) {
    m_color = QColor(skinContext.selectString(node, QStringLiteral("SignalColor")));
    m_color = WSkinColor::getCorrectColor(m_color);
}

void WaveformRendererPreroll::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(painter);
    Q_UNUSED(event);
    DEBUG_ASSERT(false);
}

void WaveformRendererPreroll::preprocess() {
    if (!preprocessInner()) {
        if (geometry().vertexCount() != 0) {
            geometry().allocate(0);
            markDirtyGeometry();
        }
    } else {
        markDirtyMaterial();
        markDirtyGeometry();
    }
}

bool WaveformRendererPreroll::preprocessInner() {
    const TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();

    if (!trackInfo || (m_isSlipRenderer && !m_waveformRenderer->isSlipActive())) {
        return false;
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
    const int numVerticesPerRectangle = 6;

    if (!preRollVisible && !postRollVisible) {
        return false;
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
        dynamic_cast<PatternMaterial&>(material())
                .setTexture(std::make_unique<Texture>(m_waveformRenderer->getContext(),
                        drawPrerollImage(m_markerLength,
                                m_markerBreadth,
                                m_waveformRenderer->getDevicePixelRatio(),
                                m_color)));
    }

    const int reservedVertexCount = (preRollVisible ? numVerticesPerRectangle : 0) +
            (postRollVisible ? numVerticesPerRectangle : 0);

    geometry().allocate(reservedVertexCount);

    const float end = m_waveformRenderer->getLength();

    TexturedVertexUpdater vertexUpdater{geometry().vertexDataAs<Geometry::TexturedPoint2D>()};

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

        const float repetitions = x / markerLength;

        vertexUpdater.addRectangle({x, halfBreadth - halfMarkerBreadth},
                {0,
                        m_isSlipRenderer ? halfBreadth
                                         : halfBreadth + halfMarkerBreadth},
                {0.f, 0.f},
                {repetitions, m_isSlipRenderer ? 0.5f : 1.f});
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

        const float repetitions = (end - x) / markerLength;

        vertexUpdater.addRectangle({x, halfBreadth - halfMarkerBreadth},
                {end,
                        m_isSlipRenderer ? halfBreadth
                                         : halfBreadth + halfMarkerBreadth},
                {0.f, 0.f},
                {repetitions, m_isSlipRenderer ? 0.5f : 1.f});
    }

    DEBUG_ASSERT(reservedVertexCount == vertexUpdater.index());

    return true;
}

} // namespace allshader
