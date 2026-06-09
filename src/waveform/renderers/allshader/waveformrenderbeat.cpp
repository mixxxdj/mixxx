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
#include "waveform/waveformwidgetfactory.h"
#include "widget/wskincolor.h"

using namespace rendergraph;

namespace allshader {

WaveformRenderBeat::WaveformRenderBeat(WaveformWidgetRenderer* waveformWidget,
        ::WaveformRendererAbstract::PositionSource type)
        : ::WaveformRendererAbstract(waveformWidget),
          m_isSlipRenderer(type == ::WaveformRendererAbstract::Slip),
          m_introStartPosCO(m_waveformRenderer->getGroup(),
                  QStringLiteral("intro_start_position")) {
    initForRectangles<UniColorMaterial>(0);
    setUsePreprocess(true);

    auto pDownbeatNode = std::make_unique<GeometryNode>();
    pDownbeatNode->initForRectangles<UniColorMaterial>(0);
    m_pDownbeatNode = appendChildNode(std::move(pDownbeatNode));

    auto* pFactory = WaveformWidgetFactory::instance();
    m_beatsPerBar = pFactory->getBeatsPerBar();
    m_downbeatsEnabled = pFactory->getDownbeatsEnabled();
    connect(pFactory,
            &WaveformWidgetFactory::beatsPerBarChanged,
            this,
            &WaveformRenderBeat::setBeatsPerBar);
    connect(pFactory,
            &WaveformWidgetFactory::downbeatsEnabledChanged,
            this,
            &WaveformRenderBeat::setDownbeatsEnabled);
}

void WaveformRenderBeat::onSetTrack() {
    if (m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(),
                &Track::beatsUpdated,
                this,
                &WaveformRenderBeat::slotBeatsUpdated);
        disconnect(m_pLoadedTrack.get(),
                &Track::cuesUpdated,
                this,
                &WaveformRenderBeat::slotCuesUpdated);
    }

    m_pLoadedTrack = m_waveformRenderer->getTrackInfo();
    m_pTrackBeats.reset();
    m_anchorBeatIndex = 0;

    if (m_pLoadedTrack) {
        m_pTrackBeats = m_pLoadedTrack->getBeats();
        connect(m_pLoadedTrack.get(),
                &Track::beatsUpdated,
                this,
                &WaveformRenderBeat::slotBeatsUpdated);
        connect(m_pLoadedTrack.get(),
                &Track::cuesUpdated,
                this,
                &WaveformRenderBeat::slotCuesUpdated);
        updateDownbeatAnchor();
    }
}

void WaveformRenderBeat::slotBeatsUpdated() {
    if (m_pLoadedTrack) {
        m_pTrackBeats = m_pLoadedTrack->getBeats();
    }
    updateDownbeatAnchor();
}

void WaveformRenderBeat::slotCuesUpdated() {
    updateDownbeatAnchor();
}

void WaveformRenderBeat::updateDownbeatAnchor() {
    m_anchorBeatIndex = 0;
    if (!m_pTrackBeats) {
        return;
    }
    const double introStartSample = m_introStartPosCO.get();
    if (introStartSample <= 0.0) {
        return;
    }
    const auto introPos = mixxx::audio::FramePos::fromEngineSamplePos(introStartSample);
    if (!introPos.isValid()) {
        return;
    }
    const auto closestBeat = m_pTrackBeats->findClosestBeat(introPos);
    if (!closestBeat.isValid()) {
        return;
    }
    const auto anchorIt = m_pTrackBeats->iteratorFrom(closestBeat);
    if (anchorIt != m_pTrackBeats->cend()) {
        m_anchorBeatIndex = anchorIt - m_pTrackBeats->cfirstmarker();
    }

    if (m_pTrackBeats->downbeatOffset() > 0) {
        m_anchorBeatIndex = m_pTrackBeats->downbeatOffset();
    }
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
    if (!m_pTrackBeats ||
            (m_isSlipRenderer && !m_waveformRenderer->isSlipActive())) {
        return false;
    }

    const double trackSamples = m_waveformRenderer->getTrackSamples();
    if (trackSamples <= 0.0) {
        return false;
    }

    auto positionType = m_isSlipRenderer ? ::WaveformRendererAbstract::Slip
                                         : ::WaveformRendererAbstract::Play;

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

    const auto firstMarker = m_pTrackBeats->cfirstmarker();

    const int beatsPerBar = (m_pTrackBeats && m_pTrackBeats->beatsPerBar() > 0)
            ? m_pTrackBeats->beatsPerBar()
            : m_beatsPerBar;

    int numRegularBeats = 0;
    int numDownbeats = 0;
    for (auto it = m_pTrackBeats->iteratorFrom(startPosition);
            it != m_pTrackBeats->cend() && *it <= endPosition;
            ++it) {
        if (m_downbeatsEnabled) {
            const int globalBeatIndex = (it - firstMarker) - m_anchorBeatIndex;
            const int mod = beatsPerBar > 0
                    ? ((globalBeatIndex % beatsPerBar) + beatsPerBar) % beatsPerBar
                    : 1;
            if (mod == 0) {
                numDownbeats++;
            } else {
                numRegularBeats++;
            }
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

    for (auto it = m_pTrackBeats->iteratorFrom(startPosition);
            it != m_pTrackBeats->cend() && *it <= endPosition;
            ++it) {
        double beatPosition = it->toEngineSamplePos();
        double xBeatPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(
                        beatPosition, positionType);

        xBeatPoint = qRound(xBeatPoint * devicePixelRatio) / devicePixelRatio;

        bool isDownbeat = false;
        if (m_downbeatsEnabled) {
            const int globalBeatIndex = (it - firstMarker) - m_anchorBeatIndex;
            const int mod = beatsPerBar > 0
                    ? ((globalBeatIndex % beatsPerBar) + beatsPerBar) % beatsPerBar
                    : 1;
            isDownbeat = (mod == 0);
        }

        const float x1 = static_cast<float>(xBeatPoint);
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
