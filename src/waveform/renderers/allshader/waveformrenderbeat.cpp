#include "waveform/renderers/allshader/waveformrenderbeat.h"

#include <QDomNode>

#include "moc_waveformrenderbeat.cpp"
#include "rendergraph/geometry.h"
#include "rendergraph/geometrynode.h"
#include "rendergraph/material/unicolormaterial.h"
#include "rendergraph/vertexupdaters/vertexupdater.h"
#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "widget/wskincolor.h"

using namespace rendergraph;

namespace allshader {

WaveformRenderBeat::WaveformRenderBeat(WaveformWidgetRenderer* waveformWidget,
        ::WaveformRendererAbstract::PositionSource type)
        : ::WaveformRendererAbstract(waveformWidget),
          m_isSlipRenderer(type == ::WaveformRendererAbstract::Slip) {
    initForRectangles<UniColorMaterial>(0);
    setUsePreprocess(true);

    auto pDownbeatNode = std::make_unique<GeometryNode>();
    pDownbeatNode->initForRectangles<UniColorMaterial>(0);
    m_pDownbeatNode = appendChildNode(std::move(pDownbeatNode));
}

void WaveformRenderBeat::setup(const QDomNode& node, const SkinContext& skinContext) {
    m_color = QColor(skinContext.selectString(node, QStringLiteral("BeatColor")));
    m_color = WSkinColor::getCorrectColor(m_color).toRgb();

    const QString downbeatColorStr = skinContext.selectString(
            node, QStringLiteral("DownbeatColor"));
    if (downbeatColorStr.isEmpty()) {
        m_downbeatColor = m_color;
    } else {
        m_downbeatColor = WSkinColor::getCorrectColor(QColor(downbeatColorStr)).toRgb();
    }
}

void WaveformRenderBeat::draw(QPainter* painter, QPaintEvent* event) {
    Q_UNUSED(painter);
    Q_UNUSED(event);
    DEBUG_ASSERT(false);
}

void WaveformRenderBeat::preprocess() {
    if (!preprocessInner()) {
        geometry().allocate(0);
        markDirtyGeometry();
        m_pDownbeatNode->geometry().allocate(0);
        m_pDownbeatNode->markDirtyGeometry();
    }
}

bool WaveformRenderBeat::preprocessInner() {
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

    QColor beatColor = m_color;
    QColor downbeatColor = m_downbeatColor;

#ifndef __SCENEGRAPH__
    int alpha = m_waveformRenderer->getBeatGridAlpha();
    if (alpha == 0) {
        return false;
    }
    beatColor.setAlphaF(alpha / 100.0f);
    downbeatColor.setAlphaF(alpha / 100.0f);
#endif

    if (!beatColor.alpha() && !downbeatColor.alpha()) {
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
    const float beatHeight = m_isSlipRenderer ? rendererBreadth / 2 : rendererBreadth;

    const int numVerticesPerLine = 6; // 2 triangles

    const auto firstMarker = trackBeats->cfirstmarker();

    // Count regular beats and downbeats separately to reserve geometry.
    // The modulo operation must handle negative indices correctly (beats
    // before the first marker). In C++, (-1 % 4) == -1, not 3, so we
    // need to normalize.
    int numRegularBeats = 0;
    int numDownbeats = 0;
    for (auto it = trackBeats->iteratorFrom(startPosition);
            it != trackBeats->cend() && *it <= endPosition;
            ++it) {
        const int globalBeatIndex = it - firstMarker;
        const int mod = m_beatsPerBar > 0
                ? ((globalBeatIndex % m_beatsPerBar) + m_beatsPerBar) % m_beatsPerBar
                : 1;
        if (mod == 0) {
            numDownbeats++;
        } else {
            numRegularBeats++;
        }
    }

    // Update regular beat geometry (on this node)
    const int reservedBeats = numRegularBeats * numVerticesPerLine;
    geometry().allocate(reservedBeats);
    VertexUpdater beatUpdater{geometry().vertexDataAs<Geometry::Point2D>()};

    // Update downbeat geometry (on child node)
    const int reservedDownbeats = numDownbeats * numVerticesPerLine;
    m_pDownbeatNode->geometry().allocate(reservedDownbeats);
    VertexUpdater downbeatUpdater{
            m_pDownbeatNode->geometry().vertexDataAs<Geometry::Point2D>()};

    for (auto it = trackBeats->iteratorFrom(startPosition);
            it != trackBeats->cend() && *it <= endPosition;
            ++it) {
        double beatPosition = it->toEngineSamplePos();
        double xBeatPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(
                        beatPosition, positionType);

        xBeatPoint = qRound(xBeatPoint * devicePixelRatio) / devicePixelRatio;

        const int globalBeatIndex = it - firstMarker;
        const int mod = m_beatsPerBar > 0
                ? ((globalBeatIndex % m_beatsPerBar) + m_beatsPerBar) % m_beatsPerBar
                : 1;
        const bool isDownbeat = (mod == 0);

        const float x1 = static_cast<float>(xBeatPoint);
        // Downbeats are 2px wide, regular beats are 1px wide
        const float x2 = x1 + (isDownbeat ? 2.f : 1.f);

        if (isDownbeat) {
            downbeatUpdater.addRectangle({x1, 0.f}, {x2, beatHeight});
        } else {
            beatUpdater.addRectangle({x1, 0.f}, {x2, beatHeight});
        }
    }

    DEBUG_ASSERT(reservedBeats == beatUpdater.index());
    DEBUG_ASSERT(reservedDownbeats == downbeatUpdater.index());

    markDirtyGeometry();
    material().setUniform(1, beatColor);
    markDirtyMaterial();

    m_pDownbeatNode->markDirtyGeometry();
    m_pDownbeatNode->material().setUniform(1, downbeatColor);
    m_pDownbeatNode->markDirtyMaterial();

    return true;
}

} // namespace allshader
