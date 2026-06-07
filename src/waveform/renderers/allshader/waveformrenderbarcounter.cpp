#include "waveform/renderers/allshader/waveformrenderbarcounter.h"

#include <QDomNode>
#include <QFont>
#include <QImage>
#include <QPainter>

#include "moc_waveformrenderbarcounter.cpp"
#include "rendergraph/context.h"
#include "rendergraph/geometry.h"
#include "rendergraph/geometrynode.h"
#include "rendergraph/material/texturematerial.h"
#include "rendergraph/texture.h"
#include "rendergraph/vertexupdaters/texturedvertexupdater.h"
#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"

using namespace rendergraph;

namespace allshader {

WaveformRenderBarCounter::WaveformRenderBarCounter(
        WaveformWidgetRenderer* waveformWidget,
        ::WaveformRendererAbstract::PositionSource type)
        : ::WaveformRendererAbstract(waveformWidget),
          m_isSlipRenderer(type == ::WaveformRendererAbstract::Slip) {
    setUsePreprocess(true);
}

void WaveformRenderBarCounter::setup(
        const QDomNode& node, const SkinContext& skinContext) {
    const QString colorStr = skinContext.selectString(
            node, QStringLiteral("BarCounterColor"));
    if (colorStr.isEmpty()) {
        m_color = QColor(255, 255, 255, 180);
    } else {
        m_color = WSkinColor::getCorrectColor(QColor(colorStr)).toRgb();
    }
}

void WaveformRenderBarCounter::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(painter);
    Q_UNUSED(event);
    DEBUG_ASSERT(false);
}

void WaveformRenderBarCounter::removeAllChildNodes() {
    while (firstChild()) {
        detachChildNode(firstChild());
    }
}

void WaveformRenderBarCounter::preprocess() {
    if (!preprocessInner()) {
        removeAllChildNodes();
    }
}

bool WaveformRenderBarCounter::preprocessInner() {
    const TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();

    if (!trackInfo || (m_isSlipRenderer && !m_waveformRenderer->isSlipActive())) {
        return false;
    }

    auto positionType = m_isSlipRenderer ? ::WaveformRendererAbstract::Slip
                                         : ::WaveformRendererAbstract::Play;

    mixxx::BeatsPointer trackBeats = trackInfo->getBeats();
    if (!trackBeats) {
        return false;
    }

    if (!m_color.alpha()) {
        return true;
    }

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();

    const double trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0.0) {
        return false;
    }

    const double firstDisplayedPosition =
            m_waveformRenderer->getFirstDisplayedPosition(positionType);
    const double lastDisplayedPosition =
            m_waveformRenderer->getLastDisplayedPosition(positionType);

    const auto startPosition = mixxx::audio::FramePos::fromEngineSamplePos(
            firstDisplayedPosition * trackSamples);
    const auto endPosition = mixxx::audio::FramePos::fromEngineSamplePos(
            lastDisplayedPosition * trackSamples);

    if (!startPosition.isValid() || !endPosition.isValid()) {
        return false;
    }

    const float rendererBreadth = m_waveformRenderer->getBreadth();
    const auto firstMarker = trackBeats->cfirstmarker();

    // Remove previous frame's nodes
    removeAllChildNodes();

    rendergraph::Context* pContext = m_waveformRenderer->getContext();
    if (!pContext) {
        return false;
    }

    // Font setup for bar number labels
    const int fontSize = static_cast<int>(10.0f * devicePixelRatio);
    QFont font;
    font.setPixelSize(fontSize);
    font.setBold(true);

    for (auto it = trackBeats->iteratorFrom(startPosition);
            it != trackBeats->cend() && *it <= endPosition;
            ++it) {
        const int globalBeatIndex = it - firstMarker;
        const int normalizedMod = m_beatsPerBar > 0
                ? ((globalBeatIndex % m_beatsPerBar) + m_beatsPerBar) % m_beatsPerBar
                : 1;

        if (normalizedMod != 0) {
            continue;
        }

        const int barNumber = (globalBeatIndex / m_beatsPerBar) + 1;

        double beatPosition = it->toEngineSamplePos();
        double xBeatPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(
                        beatPosition, positionType);
        xBeatPoint = qRound(xBeatPoint * devicePixelRatio) / devicePixelRatio;

        // Render the bar number text into a QImage
        const QString text = QString::number(barNumber);
        QFontMetrics metrics(font);
        const int textWidth = metrics.horizontalAdvance(text) + 4;
        const int textHeight = metrics.height() + 2;

        QImage image(textWidth, textHeight, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        image.setDevicePixelRatio(devicePixelRatio);

        QPainter painter(&image);
        painter.setFont(font);
        painter.setPen(m_color);
        painter.drawText(image.rect(), Qt::AlignCenter, text);
        painter.end();

        // Create a textured GeometryNode for this label
        auto pNode = std::make_unique<GeometryNode>();
        pNode->initForRectangles<TextureMaterial>(1);

        dynamic_cast<TextureMaterial&>(pNode->material())
                .setTexture(std::make_unique<Texture>(pContext, image));
        pNode->markDirtyMaterial();

        // Position the label at the bottom of the waveform
        const float x1 = static_cast<float>(xBeatPoint);
        const float labelWidth = static_cast<float>(textWidth) / devicePixelRatio;
        const float labelHeight = static_cast<float>(textHeight) / devicePixelRatio;
        const float y1 = rendererBreadth - labelHeight;
        const float x2 = x1 + labelWidth;
        const float y2 = rendererBreadth;

        TexturedVertexUpdater updater{
                pNode->geometry().vertexDataAs<Geometry::TexturedPoint2D>()};
        updater.addRectangle({x1, y1}, {x2, y2}, {0.f, 0.f}, {1.f, 1.f});
        pNode->markDirtyGeometry();

        appendChildNode(std::move(pNode));
    }

    return true;
}

} // namespace allshader
