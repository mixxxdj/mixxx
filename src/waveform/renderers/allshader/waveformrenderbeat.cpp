#include "waveform/renderers/allshader/waveformrenderbeat.h"

#include <qnamespace.h>

#include <QDomNode>
#include <iterator>

#include "moc_waveformrenderbeat.cpp"
#include "rendergraph/geometry.h"
#include "rendergraph/material/rgbamaterial.h"
#include "rendergraph/vertexupdaters/rgbavertexupdater.h"
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
    initForRectangles<RGBAMaterial>(0);
    setUsePreprocess(true);
}

void WaveformRenderBeat::setup(const QDomNode& node, const SkinContext& skinContext) {
    m_color = QColor(skinContext.selectString(node, QStringLiteral("BeatColor")));
    m_color = WSkinColor::getCorrectColor(m_color).toRgb();

    const QString downBeatColorName =
            skinContext.selectString(node, QStringLiteral("DownBeatColor"));
    m_downbeatColor = downBeatColorName.isEmpty() ? Qt::red : QColor(downBeatColorName);
    m_downbeatColor = WSkinColor::getCorrectColor(m_downbeatColor).toRgb();
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

    const TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    m_pLoadedTrack = pTrack;
    slotBeatsUpdated();

    if (!pTrack) {
        return;
    }

    connect(pTrack.get(),
            &Track::beatsUpdated,
            this,
            &WaveformRenderBeat::slotBeatsUpdated);
    connect(pTrack.get(),
            &Track::cuesUpdated,
            this,
            &WaveformRenderBeat::slotCuesUpdated);
}

void WaveformRenderBeat::slotBeatsUpdated() {
    setFirstDownbeatMaybeInvalid();
}

void WaveformRenderBeat::slotCuesUpdated() {
    setFirstDownbeatMaybeInvalid();
}

void WaveformRenderBeat::setFirstDownbeatMaybeInvalid() {
    mixxx::audio::FramePos introCuePos;
    if (!m_pLoadedTrack) {
        m_pTrackBeats.reset();
    } else {
        m_pTrackBeats = m_pLoadedTrack->getBeats();
        introCuePos = mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                m_introStartPosCO.get());
    }

    m_firstDownBeat = std::nullopt;
    if (m_pTrackBeats && introCuePos.isValid()) {
        auto introCueBeatPos = m_pTrackBeats->findClosestBeat(introCuePos);
        auto beatIt = m_pTrackBeats->iteratorFrom(introCueBeatPos);
        if (beatIt != m_pTrackBeats->cend()) {
            m_firstDownBeat = beatIt;
        }
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

#ifndef __SCENEGRAPH__
    int alpha = m_waveformRenderer->getBeatGridAlpha();
    if (alpha == 0) {
        return false;
    }
    m_color.setAlphaF(alpha / 100.0f);
    m_downbeatColor.setAlphaF(alpha / 100.0f);
#endif

    if (!m_color.alpha() && !m_downbeatColor.alpha()) {
        // Don't render the beatgrid lines is there are fully transparent
        return true;
    }

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();

    auto positionType = m_isSlipRenderer ? ::WaveformRendererAbstract::Slip
                                         : ::WaveformRendererAbstract::Play;
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

    // Count the number of beats in the range to reserve space in the m_vertices vector.
    // Note that we could also use
    //   int numBearsInRange = m_pTrackBeats->numBeatsInRange(startPosition, endPosition);
    // for this, but there have been reports of that method failing with a DEBUG_ASSERT.
    int numBeatsInRange = 0;
    for (auto it = m_pTrackBeats->iteratorFrom(startPosition);
            it != m_pTrackBeats->cend() && *it <= endPosition;
            ++it) {
        numBeatsInRange++;
    }

    const int reserved = numBeatsInRange * numVerticesPerLine;
    geometry().allocate(reserved);

    RGBAVertexUpdater vertexUpdater{geometry().vertexDataAs<Geometry::RGBAColoredPoint2D>()};

    float beat_r = m_color.redF(), beat_g = m_color.greenF(),
          beat_b = m_color.blueF(), beat_alpha = m_color.alphaF();
    float downbeat_r = m_downbeatColor.redF(),
          downbeat_g = m_downbeatColor.greenF(),
          downbeat_b = m_downbeatColor.blueF(),
          downbeat_alpha = m_downbeatColor.alphaF();

    std::optional<mixxx::Beats::ConstIterator> firstDownbeat = std::nullopt;
    auto* pWaveformWidgetFactory = WaveformWidgetFactory::instance();
    bool downbeatsEnabled = false;
    int downbeatDistance = pWaveformWidgetFactory->getDownbeatDistance();
    if (pWaveformWidgetFactory->getDownbeatsEnabled() && m_firstDownBeat.has_value()) {
        downbeatsEnabled = true;
        firstDownbeat = *m_firstDownBeat;
    }

    for (auto it = m_pTrackBeats->iteratorFrom(startPosition);
            it != m_pTrackBeats->cend() && *it <= endPosition;
            ++it) {
        double beatPosition = it->toEngineSamplePos();
        double xBeatPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(
                        beatPosition, positionType);

        xBeatPoint = qRound(xBeatPoint * devicePixelRatio) / devicePixelRatio;

        const float x1 = static_cast<float>(xBeatPoint);
        const float x2 = x1 + 1.f;

        if (downbeatsEnabled && std::distance(*firstDownbeat, it) % downbeatDistance == 0) {
            vertexUpdater.addRectangleHGradient({x1, 0.f},
                    {x2, m_isSlipRenderer ? rendererBreadth / 2 : rendererBreadth},
                    {downbeat_r, downbeat_g, downbeat_b, downbeat_alpha},
                    {downbeat_r, downbeat_g, downbeat_b, downbeat_alpha});
        } else {
            vertexUpdater.addRectangleHGradient({x1, 0.f},
                    {x2, m_isSlipRenderer ? rendererBreadth / 2 : rendererBreadth},
                    {beat_r, beat_g, beat_b, beat_alpha},
                    {beat_r, beat_g, beat_b, beat_alpha});
        }
    }
    markDirtyGeometry();

    DEBUG_ASSERT(reserved == vertexUpdater.index());

    markDirtyMaterial();

    return true;
}

} // namespace allshader
