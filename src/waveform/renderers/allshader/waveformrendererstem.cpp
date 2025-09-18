#include "waveform/renderers/allshader/waveformrendererstem.h"

#include <QFont>
#include <QImage>
#include <QOpenGLTexture>

#include "control/controlproxy.h"
#include "engine/channels/enginedeck.h"
#include "engine/engine.h"
#include "rendergraph/material/rgbamaterial.h"
#include "rendergraph/vertexupdaters/rgbavertexupdater.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/math.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"

namespace {
#ifdef __SCENEGRAPH__
// FIXME this is a workaround an issue with waveform only drawing partially in
// SG. The workaround is to reduce the the number of vertices, by reducing the
// precision of waveform strips.
const float kPixelPerStrip = 2;
#else
const float kPixelPerStrip = 1;
#endif
} // namespace

using namespace rendergraph;

namespace allshader {

WaveformRendererStem::WaveformRendererStem(
        WaveformWidgetRenderer* waveformWidget,
        ::WaveformRendererAbstract::PositionSource type,
        ::WaveformRendererSignalBase::Options options)
        : WaveformRendererSignalBase(waveformWidget, options),
          m_isSlipRenderer(type == ::WaveformRendererAbstract::Slip),
          m_splitStemTracks(false),
          m_outlineOpacity(0.15f),
          m_opacity(0.75f) {
    initForRectangles<RGBAMaterial>(0);
    setUsePreprocess(true);
}

void WaveformRendererStem::onSetup(const QDomNode&) {
}

bool WaveformRendererStem::init() {
    m_pStemGain.clear();
    m_pStemMute.clear();
    if (m_waveformRenderer->getGroup().isEmpty()) {
        return true;
    }
    for (int stemIdx = 0; stemIdx < mixxx::kMaxSupportedStems; stemIdx++) {
        QString stemGroup = EngineDeck::getGroupForStem(m_waveformRenderer->getGroup(), stemIdx);
        m_pStemGain.emplace_back(
                std::make_unique<ControlProxy>(stemGroup,
                        QStringLiteral("volume")));
        m_pStemMute.emplace_back(
                std::make_unique<ControlProxy>(stemGroup,
                        QStringLiteral("mute")));
        auto bringToForeground = [this, stemIdx](double) {
            if (!m_reorderOnChange) {
                return;
            }
            m_stackOrder.removeAll(stemIdx);
            m_stackOrder.append(stemIdx);
        };
        m_pStemGain.back()->connectValueChanged(this, bringToForeground);
        m_pStemMute.back()->connectValueChanged(this, bringToForeground);
    }

    m_stackOrder.resize(mixxx::kMaxSupportedStems);
    std::iota(m_stackOrder.begin(), m_stackOrder.end(), 0);

#ifndef __SCENEGRAPH__
    auto* pWaveformWidgetFactory = WaveformWidgetFactory::instance();
    setReorderOnChange(pWaveformWidgetFactory->isStemReorderOnChange());
    connect(pWaveformWidgetFactory,
            &WaveformWidgetFactory::stemReorderOnChangeChanged,
            this,
            &WaveformRendererStem::setReorderOnChange);
    setOutlineOpacity(pWaveformWidgetFactory->getStemOutlineOpacity());
    connect(pWaveformWidgetFactory,
            &WaveformWidgetFactory::stemOutlineOpacityChanged,
            this,
            &WaveformRendererStem::setOutlineOpacity);
    setOpacity(pWaveformWidgetFactory->getStemOpacity());
    connect(pWaveformWidgetFactory,
            &WaveformWidgetFactory::stemOpacityChanged,
            this,
            &WaveformRendererStem::setOpacity);
#endif
    return true;
}

void WaveformRendererStem::preprocess() {
    if (!preprocessInner()) {
        if (geometry().vertexCount() != 0) {
            geometry().allocate(0);
            markDirtyGeometry();
        }
    }
}

bool WaveformRendererStem::preprocessInner() {
    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack || (m_isSlipRenderer && !m_waveformRenderer->isSlipActive())) {
        return false;
    }

    auto stemInfo = pTrack->getStemInfo();
    // If this track isn't a stem track, skip the rendering
    if (stemInfo.isEmpty()) {
        return false;
    }
    // put directly in firstVisualFrame & lastVisualFrame
    // auto positionType = m_isSlipRenderer ? ::WaveformRendererAbstract::Slip
    //                                     : ::WaveformRendererAbstract::Play;

    ConstWaveformPointer waveform = pTrack->getWaveform();
    if (waveform.isNull()) {
        return false;
    }

    const WaveformData* data = waveform->data();
    // combined
    // if (!data) {
    //    return false;
    //}
    //// If this waveform doesn't contain stem data, skip the rendering
    // if (!waveform->hasStem()) {
    //     return false;
    // }
    if (!waveform || waveform->getDataSize() <= 1 || !waveform->hasStem()) {
        return false;
    }

    uint selectedStems = m_waveformRenderer->getSelectedStems();

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    const int length = static_cast<int>(m_waveformRenderer->getLength());
    const int pixelLength = static_cast<int>(length * devicePixelRatio);
    const int stripLength = static_cast<int>(static_cast<float>(pixelLength) / kPixelPerStrip);
    const float invDevicePixelRatio = kPixelPerStrip / devicePixelRatio;
    const float halfStripSize = kPixelPerStrip / 2.0f / devicePixelRatio;

    // See waveformrenderersimple.cpp for a detailed explanation of the frame and index calculation

    // Per-band gain from the EQ knobs.
    float allGain = 1.0f;
    // applyCompensation = false, as we scale to match filtered.all
    getGains(&allGain, false, nullptr, nullptr, nullptr);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth());
    const float stemBreadth = m_splitStemTracks ? breadth / 4.0f : 0;
    const float halfBreadth = (m_splitStemTracks ? stemBreadth : breadth) / 2.0f;

    const float heightFactor = allGain * halfBreadth / m_maxValue;

    const int dataSize = waveform->getDataSize();
    const double visualFramesSize = dataSize / 2.0;

    const double firstVisualFrame =
            m_waveformRenderer->getFirstDisplayedPosition(
                    m_isSlipRenderer ? ::WaveformRendererAbstract::Slip
                                     : ::WaveformRendererAbstract::Play) *
            visualFramesSize;
    const double lastVisualFrame =
            m_waveformRenderer->getLastDisplayedPosition(
                    m_isSlipRenderer ? ::WaveformRendererAbstract::Slip
                                     : ::WaveformRendererAbstract::Play) *
            visualFramesSize;

    // Represents the # of visual frames per horizontal pixel.
    const double visualIncrementPerPixel = (lastVisualFrame - firstVisualFrame) / stripLength;
    // Effective visual frame for x
    double xVisualFrame = qRound(firstVisualFrame / visualIncrementPerPixel) *
            visualIncrementPerPixel;

    const int numVerticesPerLine = 6; // 2 triangles per rectangle

    // Only allocate for actual stems, premix is NOT drawn
    // const int stemsToDraw = static_cast<int>(stemInfo.size());
    // const int reserved = numVerticesPerLine * (stemsToDraw * stripLength);
    // skip premix
    const int numStemsToDraw = mixxx::audio::ChannelCount::stem() - 1;
    // outline + fill
    const int layersPerStem = 2;
    const int reserved = numVerticesPerLine * layersPerStem * numStemsToDraw * stripLength;
    geometry().allocate(reserved);

    geometry().setDrawingMode(Geometry::DrawingMode::Triangles);
    geometry().allocate(reserved);
    markDirtyGeometry();

    RGBAVertexUpdater vertexUpdater{geometry().vertexDataAs<Geometry::RGBAColoredPoint2D>()};
    vertexUpdater.addRectangle({0.f, halfBreadth - 0.5f},
            {static_cast<float>(length),
                    m_isSlipRenderer ? halfBreadth : halfBreadth + 0.5f},
            {0.f, 0.f, 0.f, 0.f});

    const double maxSamplingRange = visualIncrementPerPixel / 2.0;
    const bool premixMuted =
            (!m_pStemMute.empty() && m_pStemMute.size() > 0) ? m_pStemMute[0]->toBool() : false;

    for (int visualIdx = 0; visualIdx < stripLength; ++visualIdx) {
        int stemLayer = 0;
        // PreMix mute state (index 0 = Stem1 = premix)
        for (int stemIdx : std::as_const(m_stackOrder)) {
            // Skip premix (stemIdx == 0)
            // Stem is drawn twice with different opacity level, this allow to
            // see the maximum signal by transparency
            // Skip premix entirely; we never draw it
            if (stemIdx == 0)
                continue;

            // Map to stemInfo index (0-3) from internal (1-4)
            const int colorIdx = stemIdx - 1;
            if (colorIdx < 0 || colorIdx >= static_cast<int>(stemInfo.size()))
                continue;

            // Colour for this stem
            const QColor stemColor = stemInfo[colorIdx].getColor();
            const float color_r = stemColor.redF();
            const float color_g = stemColor.greenF();
            const float color_b = stemColor.blueF();
            const float color_a_base = stemColor.alphaF();

            // Window of samples contributing to this pixel column
            const int visualFrameStart = std::lround(xVisualFrame - maxSamplingRange);
            const int visualFrameStop = std::lround(xVisualFrame + maxSamplingRange);
            const int visualIndexStart = std::max(visualFrameStart * 2, 0);
            const int visualIndexStop = std::min(
                    std::max(visualFrameStop, visualFrameStart + 1) * 2,
                    waveform->getDataSize() - 1);

            const float fVisualIdx = static_cast<float>(visualIdx) * invDevicePixelRatio;

            // Find the max values for current eq in the waveform data.
            // - Max of left and right
            uchar u8max = 0;
            for (int chn = 0; chn < 2; ++chn) {
                // data is interleaved left / right
                for (int i = visualIndexStart + chn; i < visualIndexStop + chn; i += 2) {
                    // waveformData.stems[] is 0-4, where 0 is premix, 1-4 the stems
                    u8max = math_max(u8max, data[i].stems[stemIdx]);
                }
            }

            // Cast to float
            const float max = static_cast<float>(u8max);

            // Two layers (outline + fill) for stems not for PreMix
            for (int layerIdx = 0; layerIdx < 2; ++layerIdx) {
                float color_a = color_a_base * (layerIdx ? m_opacity : m_outlineOpacity);

                // Apply the gains -> effective gain:
                // - If premix is unmuted -> show all stems at 100% (bright)
                // - Else follow this stem’s mute/gain and selection mask
                float effectiveGain = 1.0f;
                if (premixMuted) {
                    const bool isMuted = (m_pStemMute.size() > static_cast<size_t>(stemIdx))
                            ? m_pStemMute[stemIdx]->toBool()
                            : false;
                    const float volume = (m_pStemGain.size() > static_cast<size_t>(stemIdx))
                            ? static_cast<float>(m_pStemGain[stemIdx]->get())
                            : 1.0f;
                    const bool deselected = selectedStems && !(selectedStems & (1 << stemIdx));
                    effectiveGain = (isMuted || deselected) ? 0.0f : volume;
                }

                const float h = heightFactor * (max * effectiveGain);
                // Lines are thin rectangles
                // shadow
                vertexUpdater.addRectangle(
                        {fVisualIdx - halfStripSize,
                                stemLayer * stemBreadth + halfBreadth - h},
                        {fVisualIdx + halfStripSize,
                                stemLayer * stemBreadth + halfBreadth +
                                        (m_isSlipRenderer ? 0.f : h)},
                        {color_r, color_g, color_b, color_a});
            }
            ++stemLayer;
        }
        xVisualFrame += visualIncrementPerPixel;
    }

    markDirtyMaterial();
    DEBUG_ASSERT(vertexUpdater.index() <= reserved);
    return true;
}
} // namespace allshader
