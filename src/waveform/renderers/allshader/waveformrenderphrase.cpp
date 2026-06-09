#include "waveform/renderers/allshader/waveformrenderphrase.h"

#include "rendergraph/geometry.h"
#include "rendergraph/geometrynode.h"
#include "rendergraph/material/unicolormaterial.h"
#include "rendergraph/node.h"
#include "rendergraph/vertexupdaters/vertexupdater.h"
#include "track/phrases.h"
#include "track/track.h"
#include "waveform/renderers/waveformwidgetrenderer.h"

using namespace rendergraph;

namespace allshader {

WaveformRenderPhrase::WaveformRenderPhrase(
        WaveformWidgetRenderer* waveformWidget,
        ::WaveformRendererAbstract::PositionSource type)
        : ::WaveformRendererAbstract(waveformWidget),
          m_isSlipRenderer(type == ::WaveformRendererAbstract::Slip) {
    setUsePreprocess(true);
}

void WaveformRenderPhrase::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(painter);
    Q_UNUSED(event);
    DEBUG_ASSERT(false);
}

void WaveformRenderPhrase::setup(
        const QDomNode& node, const SkinContext& skinContext) {
    Q_UNUSED(node);
    Q_UNUSED(skinContext);
}

void WaveformRenderPhrase::preprocess() {
    if (!preprocessInner()) {
        while (firstChild()) {
            detachChildNode(firstChild());
        }
    }
}

bool WaveformRenderPhrase::preprocessInner() {
    const TrackPointer pTrackInfo = m_waveformRenderer->getTrackInfo();
    if (!pTrackInfo || (m_isSlipRenderer && !m_waveformRenderer->isSlipActive())) {
        return false;
    }

    auto positionType = m_isSlipRenderer ? ::WaveformRendererAbstract::Slip
                                         : ::WaveformRendererAbstract::Play;

    mixxx::PhrasesPointer pPhrases = pTrackInfo->getPhrases();
    if (!pPhrases || pPhrases->isEmpty()) {
        return false;
    }

    const double trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0.0) {
        return false;
    }

    const double firstDisplayedPosition =
            m_waveformRenderer->getFirstDisplayedPosition(positionType);
    const double lastDisplayedPosition =
            m_waveformRenderer->getLastDisplayedPosition(positionType);

    while (firstChild()) {
        detachChildNode(firstChild());
    }

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth());

    for (const auto& phrase : pPhrases->phrases()) {
        const double startSample = phrase.startPosition().toEngineSamplePos();
        const double endSample = phrase.endPosition().toEngineSamplePos();

        const double startRatio = startSample / trackSamples;
        const double endRatio = endSample / trackSamples;

        if (startRatio > lastDisplayedPosition || endRatio < firstDisplayedPosition) {
            continue;
        }

        double startX = m_waveformRenderer->transformSamplePositionInRendererWorld(
                startSample, positionType);
        double endX = m_waveformRenderer->transformSamplePositionInRendererWorld(
                endSample, positionType);

        startX = std::max(0.0, std::floor(startX));
        endX = std::min(static_cast<double>(m_waveformRenderer->getLength()),
                std::floor(endX));

        if (endX <= startX) {
            continue;
        }

        QColor color = mixxx::Phrase::defaultColor(phrase.type());

        auto pNode = std::make_unique<GeometryNode>();
        pNode->initForRectangles<UniColorMaterial>(1);

        VertexUpdater vertexUpdater{
                pNode->geometry().vertexDataAs<Geometry::Point2D>()};
        vertexUpdater.addRectangle(
                {static_cast<float>(startX), 0.f},
                {static_cast<float>(endX), breadth});
        pNode->material().setUniform(1, color);
        pNode->markDirtyGeometry();
        pNode->markDirtyMaterial();

        appendChildNode(std::move(pNode));
    }

    return true;
}

} // namespace allshader
