#include "waveform/renderers/allshader/waveformrenderdownbeat.h"

#include <QDomNode>

#include "moc_waveformrenderdownbeat.cpp"
#include "rendergraph/geometry.h"
#include "rendergraph/material/unicolormaterial.h"
#include "rendergraph/vertexupdaters/vertexupdater.h"
#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"

using namespace rendergraph;

namespace allshader {

WaveformRenderDownBeat::WaveformRenderDownBeat(WaveformWidgetRenderer* waveformWidget,
        ::WaveformRendererAbstract::PositionSource type)
        : ::WaveformRendererAbstract(waveformWidget),
          m_isSlipRenderer(type == ::WaveformRendererAbstract::Slip) {
    initForRectangles<UniColorMaterial>(0);
    setUsePreprocess(true);
}

void WaveformRenderDownBeat::setup(const QDomNode& node, const SkinContext& skinContext) {
    m_color = QColor(skinContext.selectString(node, QStringLiteral("DownbeatColor")));
    m_color = WSkinColor::getCorrectColor(m_color).toRgb();
}

void WaveformRenderDownBeat::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(painter);
    Q_UNUSED(event);
    DEBUG_ASSERT(false);
}

void WaveformRenderDownBeat::preprocess() {
    if (!preprocessInner()) {
        geometry().allocate(0);
        markDirtyGeometry();
    }
}

bool WaveformRenderDownBeat::preprocessInner() {
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

    int alpha = m_waveformRenderer->getBeatGridAlpha();
    if (alpha == 0) {
        return false;
    }

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();

    m_color.setAlphaF(alpha / 100.0f);

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

    const int numVerticesPerLine = 6; // 2 triangles

    int downbeatOffset = trackInfo->getDownbeatOffset();

    int numDownbeatsInRange = 0;
    for (auto it = trackBeats->iteratorFrom(startPosition);
            it != trackBeats->cend() && *it <= endPosition;
            ++it) {
        if ((it.beatOffset() - downbeatOffset) % 4 != 0) {
            continue;
        }
        numDownbeatsInRange++;
    }

    const int reserved = (numDownbeatsInRange * 3) + (numDownbeatsInRange * numVerticesPerLine);
    geometry().allocate(reserved);

    VertexUpdater vertexUpdater{geometry().vertexDataAs<Geometry::Point2D>()};

    for (auto it = trackBeats->iteratorFrom(startPosition);
            it != trackBeats->cend() && *it <= endPosition;
            ++it) {
        if ((it.beatOffset() - downbeatOffset) % 4 != 0) {
            continue;
        }
        double beatPosition = it->toEngineSamplePos();
        double xBeatPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(
                        beatPosition, positionType);

        xBeatPoint = qRound(xBeatPoint * devicePixelRatio) / devicePixelRatio;

        const float x1 = static_cast<float>(xBeatPoint) + 0.f;
        const float x2 = x1 + 1.f;

        vertexUpdater.addRectangle({x1, 0.f},
                {x2, m_isSlipRenderer ? rendererBreadth / 2 : rendererBreadth});
        if ((it.beatOffset() - downbeatOffset) % 8 == 0) {
            vertexUpdater.addTriangle({x1 - 4.f, 0.f}, {x2 + 4.f, 0.f}, {x1, 6.f});
        } else {
            vertexUpdater.addTriangle({0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f});
        }
    }
    markDirtyGeometry();

    DEBUG_ASSERT(reserved == vertexUpdater.index());

    material().setUniform(1, m_color);
    markDirtyMaterial();

    return true;
}

} // namespace allshader
