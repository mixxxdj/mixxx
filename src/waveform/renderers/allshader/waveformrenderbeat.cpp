#include "waveform/renderers/allshader/waveformrenderbeat.h"

#include <qnamespace.h>

#include <QDomNode>
#include <iterator>

#include "engine/engine.h"
#include "moc_waveformrenderbeat.cpp"
#include "rendergraph/geometry.h"
#include "rendergraph/material/rgbamaterial.h"
#include "rendergraph/vertexupdaters/rgbavertexupdater.h"
#include "rendergraph/vertexupdaters/vertexupdater.h"
#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"
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

    const TrackPointer trackInfo = m_waveformRenderer->getTrackInfo();

    const bool isStemTrack = trackInfo && trackInfo->hasStem() &&
            trackInfo->getWaveform() && trackInfo->getWaveform()->hasStem();
    const bool splitStemTracks = isStemTrack &&
            WaveformWidgetFactory::instance()->isStemSplitTracks();

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
    const int numTriangleVertices = 3;

    std::optional<mixxx::Beats::ConstIterator> firstDownbeat = std::nullopt;
    auto* pWaveformWidgetFactory = WaveformWidgetFactory::instance();
    bool downbeatsEnabled = false;
    const int kDownbeatDistance = 4; // downbeats are always one bar (4 beats) apart
    const int phraseLength = pWaveformWidgetFactory->getPhraseLength();
    if (pWaveformWidgetFactory->getDownbeatsEnabled() && m_firstDownBeat.has_value()) {
        downbeatsEnabled = true;
        firstDownbeat = *m_firstDownBeat;
    }

    const int numBoxesPerBeat = (m_isSlipRenderer && splitStemTracks)
            ? mixxx::kMaxSupportedStems
            : 1;

    // Count the number of beats in the range to reserve space in the m_vertices vector.
    // Note that we could also use
    //   int numBearsInRange = m_pTrackBeats->numBeatsInRange(startPosition, endPosition);
    // for this, but there have been reports of that method failing with a DEBUG_ASSERT.
    int reserved = 0;
    for (auto it = m_pTrackBeats->iteratorFrom(startPosition);
            it != m_pTrackBeats->cend() && *it <= endPosition;
            ++it) {
        const bool isPhrase = downbeatsEnabled && phraseLength > 0 &&
                std::distance(*firstDownbeat, it) % phraseLength == 0;
        // Phrase beats get a triangle at the top and bottom edge,
        // in slip mode we need only one triengle phase mark
        const int numPhraseTriangles =
                isPhrase ? (m_isSlipRenderer ? 1 : 2) : 0;
        const int verticesPerBeat =
                numVerticesPerLine + numPhraseTriangles * numTriangleVertices;
        reserved += verticesPerBeat * numBoxesPerBeat;
    }

    geometry().allocate(reserved);

    RGBAVertexUpdater vertexUpdater{geometry().vertexDataAs<Geometry::RGBAColoredPoint2D>()};

    float beat_r = m_color.redF(), beat_g = m_color.greenF(),
          beat_b = m_color.blueF(), beat_alpha = m_color.alphaF();
    float downbeat_r = m_downbeatColor.redF(),
          downbeat_g = m_downbeatColor.greenF(),
          downbeat_b = m_downbeatColor.blueF(),
          downbeat_alpha = m_downbeatColor.alphaF();

    const float boxBreadth = splitStemTracks
            ? rendererBreadth / static_cast<float>(mixxx::kMaxSupportedStems)
            : rendererBreadth;

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

        const bool isDownbeat = downbeatsEnabled &&
                std::distance(*firstDownbeat, it) % kDownbeatDistance == 0;
        const bool isPhrase = downbeatsEnabled && phraseLength > 0 &&
                std::distance(*firstDownbeat, it) % phraseLength == 0;
        const QVector4D downbeatColor =
                QVector4D{downbeat_r, downbeat_g, downbeat_b, downbeat_alpha};
        const QVector4D beatLineColor = isDownbeat
                ? downbeatColor
                : QVector4D{beat_r, beat_g, beat_b, beat_alpha};

        const float xMid = (x1 + x2) / 2.f;
        const float triangleHalfWidth = 4.f;
        const float triangleHeight = 4.f;

        if (m_isSlipRenderer && splitStemTracks) {
            for (int stemIdx = 0; stemIdx < mixxx::kMaxSupportedStems; ++stemIdx) {
                const float posy1 = stemIdx * boxBreadth;
                const float posy2 = posy1 + boxBreadth / 2.f;
                vertexUpdater.addRectangle({x1, posy1}, {x2, posy2}, beatLineColor);
            }
        } else {
            const float lineBottom =
                    m_isSlipRenderer ? rendererBreadth / 2.f : rendererBreadth;
            vertexUpdater.addRectangle({x1, 0.f}, {x2, lineBottom}, beatLineColor);
        }

        if (isPhrase) {
            vertexUpdater.addTriangle(
                    {xMid - triangleHalfWidth, 0.f},
                    {xMid + triangleHalfWidth, 0.f},
                    {xMid, triangleHeight},
                    downbeatColor);
            if (!m_isSlipRenderer) {
                vertexUpdater.addTriangle(
                        {xMid - triangleHalfWidth, rendererBreadth},
                        {xMid + triangleHalfWidth, rendererBreadth},
                        {xMid, rendererBreadth - triangleHeight},
                        downbeatColor);
            }
        }
    }
    markDirtyGeometry();

    DEBUG_ASSERT(reserved == vertexUpdater.index());

    markDirtyMaterial();

    return true;
}

} // namespace allshader
