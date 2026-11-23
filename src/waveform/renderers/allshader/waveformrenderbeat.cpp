#include "waveform/renderers/allshader/waveformrenderbeat.h"

#include <qnamespace.h>

#include <QDomNode>
#include <QVarLengthArray>
#include <algorithm>

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

    // e.g. <BarLineWidth>, <BarMarkerShape> (triangle, tick or block),
    // <BarMarkerSize>, <BarMarkerColor> and the same with "Beat"
    auto readStyle = [&](const QString& prefix, const QColor& lineColor) {
        LineStyle style;
        style.lineWidth = std::max(0.f,
                skinContext.selectFloat(node, prefix + QStringLiteral("LineWidth"), style.lineWidth));
        const QString shape = skinContext.selectString(node, prefix + QStringLiteral("MarkerShape"));
        if (shape == QLatin1String("triangle")) {
            style.markerShape = MarkerShape::Triangle;
        } else if (shape == QLatin1String("tick")) {
            style.markerShape = MarkerShape::Tick;
        } else if (shape == QLatin1String("block")) {
            style.markerShape = MarkerShape::Block;
        }
        style.markerSize = std::max(0.f,
                skinContext.selectFloat(
                        node, prefix + QStringLiteral("MarkerSize"), style.markerSize));
        const QColor color = skinContext.selectColor(node, prefix + QStringLiteral("MarkerColor"));
        // default: the line color, opaque (the grid opacity is applied when drawing)
        style.markerColor = color.isValid() ? WSkinColor::getCorrectColor(color).toRgb()
                                            : QColor(lineColor.rgb());
        return style;
    };
    m_beatStyle = readStyle(QStringLiteral("Beat"), m_color);
    m_barStyle = readStyle(QStringLiteral("Bar"), m_downbeatColor);
    // <MarkerEdges>: top, bottom or both (default)
    const QString edges = skinContext.selectString(node, QStringLiteral("MarkerEdges"));
    m_markersOnTop = edges != QLatin1String("bottom");
    m_markersOnBottom = edges != QLatin1String("top");
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
    const bool splitStemTracks = isStemTrack && WaveformWidgetFactory::isCreated() &&
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
        return false;
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

    auto* pWaveformWidgetFactory = WaveformWidgetFactory::instance();
    const bool downbeatsEnabled =
            pWaveformWidgetFactory->getDownbeatsEnabled() && m_firstDownBeat.has_value();
    const int downbeatDistance = std::max(1, pWaveformWidgetFactory->getDownbeatDistance());
    // Position of the first visible beat in its bar. Iterator distance walks
    // the beat markers, so it is computed once here and counted up per beat.
    const auto startIt = m_pTrackBeats->iteratorFrom(startPosition);
    int firstBeatInBar = 0;
    if (downbeatsEnabled) {
        const int index = startIt - *m_firstDownBeat;
        firstBeatInBar = (index % downbeatDistance + downbeatDistance) % downbeatDistance;
    }

    const int numBoxesPerBeat = (m_isSlipRenderer && splitStemTracks)
            ? mixxx::kMaxSupportedStems
            : 1;
    // Markers sit on the top and/or bottom edge of the lines, pointing inwards.
    // The per-stem lines of the slip renderer have no common edges for them.
    QVarLengthArray<std::pair<float, float>, 2> markerEdges; // position, direction
    if (numBoxesPerBeat == 1) {
        if (m_markersOnTop) {
            markerEdges.append({0.f, 1.f});
        }
        if (m_markersOnBottom) {
            markerEdges.append({m_isSlipRenderer ? rendererBreadth / 2 : rendererBreadth, -1.f});
        }
    }
    auto numVertices = [&](const LineStyle& style) {
        const int line = style.lineWidth > 0.f ? 6 * numBoxesPerBeat : 0;
        if (style.markerShape == MarkerShape::None) {
            return line;
        }
        const int marker = style.markerShape == MarkerShape::Triangle ? 3 : 6;
        return line + marker * static_cast<int>(markerEdges.size());
    };
    const int numVerticesPerBeat = numVertices(m_beatStyle);
    const int numVerticesPerBar = numVertices(m_barStyle);

    // Count the vertices of the beats in the range to reserve space in the m_vertices vector.
    // Note that we could also use
    //   int numBearsInRange = m_pTrackBeats->numBeatsInRange(startPosition, endPosition);
    // for this, but there have been reports of that method failing with a DEBUG_ASSERT.
    int reserved = 0;
    int beatInBar = firstBeatInBar;
    for (auto it = startIt; it != m_pTrackBeats->cend() && *it <= endPosition;
            ++it) {
        reserved += downbeatsEnabled && beatInBar == 0 ? numVerticesPerBar : numVerticesPerBeat;
        beatInBar = (beatInBar + 1) % downbeatDistance;
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

    auto rgba = [](const QColor& color, float alpha) {
        return QVector4D(color.redF(), color.greenF(), color.blueF(), color.alphaF() * alpha);
    };
    // the marker colors take the beat grid opacity from the line colors
    const QVector4D beatMarkerColor = rgba(m_beatStyle.markerColor, m_color.alphaF());
    const QVector4D barMarkerColor = rgba(m_barStyle.markerColor, m_downbeatColor.alphaF());

    beatInBar = firstBeatInBar;
    for (auto it = startIt; it != m_pTrackBeats->cend() && *it <= endPosition;
            ++it) {
        double beatPosition = it->toEngineSamplePos();
        double xBeatPoint =
                m_waveformRenderer->transformSamplePositionInRendererWorld(
                        beatPosition, positionType);

        xBeatPoint = qRound(xBeatPoint * devicePixelRatio) / devicePixelRatio;

        const bool isDownbeat = downbeatsEnabled && beatInBar == 0;
        beatInBar = (beatInBar + 1) % downbeatDistance;
        const LineStyle& style = isDownbeat ? m_barStyle : m_beatStyle;

        // a 1 px line covers [x, x + 1], wider lines are centered on it
        const float x = static_cast<float>(xBeatPoint);
        const float x1 = x - (style.lineWidth - 1.f) / 2.f;
        const float x2 = x1 + style.lineWidth;

        const QVector4D color = isDownbeat
                ? QVector4D{downbeat_r, downbeat_g, downbeat_b, downbeat_alpha}
                : QVector4D{beat_r, beat_g, beat_b, beat_alpha};

        if (style.lineWidth <= 0.f) {
            // markers only
        } else if (m_isSlipRenderer && splitStemTracks) {
            for (int stemIdx = 0; stemIdx < mixxx::kMaxSupportedStems; ++stemIdx) {
                const float posy1 = stemIdx * boxBreadth;
                const float posy2 = posy1 + boxBreadth / 2.f;
                vertexUpdater.addRectangle({x1, posy1}, {x2, posy2}, color);
            }
        } else {
            vertexUpdater.addRectangle({x1, 0.f},
                    {x2, m_isSlipRenderer ? rendererBreadth / 2 : rendererBreadth},
                    color);
        }
        if (style.markerShape != MarkerShape::None) {
            const QVector4D& markerColor = isDownbeat ? barMarkerColor : beatMarkerColor;
            const float cx = x + 0.5f;
            const float halfWidth = style.markerShape == MarkerShape::Tick ? 1.f : style.markerSize;
            for (const auto& [edge, direction] : markerEdges) {
                const float tip = edge + direction * style.markerSize * 1.2f;
                if (style.markerShape == MarkerShape::Triangle) {
                    vertexUpdater.addTriangle(
                            {cx - halfWidth, edge}, {cx + halfWidth, edge}, {cx, tip}, markerColor);
                } else {
                    vertexUpdater.addRectangle({cx - halfWidth, std::min(edge, tip)},
                            {cx + halfWidth, std::max(edge, tip)},
                            markerColor);
                }
            }
        }
    }
    markDirtyGeometry();

    DEBUG_ASSERT(reserved == vertexUpdater.index());

    markDirtyMaterial();

    return true;
}

} // namespace allshader
