#include "waveform/renderers/allshader/waveformrendererstem.h"

#include <QFont>
#include <QImage>
#include <QOpenGLTexture>

#include "engine/channels/enginedeck.h"
#include "engine/engine.h"
#include "rendergraph/material/rgbamaterial.h"
#include "rendergraph/vertexupdaters/rgbavertexupdater.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/math.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"

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
        ::WaveformRendererAbstract::PositionSource type)
        : WaveformRendererSignalBase(waveformWidget),
          m_isSlipRenderer(type == ::WaveformRendererAbstract::Slip),
          m_splitStemTracks(false) {
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
                std::make_unique<PollingControlProxy>(stemGroup,
                        QStringLiteral("volume")));
        m_pStemMute.emplace_back(
                std::make_unique<PollingControlProxy>(stemGroup,
                        QStringLiteral("mute")));
    }
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
    auto positionType = m_isSlipRenderer ? ::WaveformRendererAbstract::Slip
                                         : ::WaveformRendererAbstract::Play;

    ConstWaveformPointer waveform = pTrack->getWaveform();
    if (waveform.isNull()) {
        return false;
    }

    const int dataSize = waveform->getDataSize();
    if (dataSize <= 1) {
        return false;
    }

    const WaveformData* data = waveform->data();
    if (data == nullptr) {
        return false;
    }
    // If this waveform doesn't contain stem data, skip the rendering
    if (!waveform->hasStem()) {
        return false;
    }

    uint selectedStems = m_waveformRenderer->getSelectedStems();

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    const int length = static_cast<int>(m_waveformRenderer->getLength());
    const int pixelLength = static_cast<int>(m_waveformRenderer->getLength() * devicePixelRatio);
    const int stripLength = static_cast<int>(static_cast<float>(pixelLength) / kPixelPerStrip);
    const float invDevicePixelRatio = kPixelPerStrip / devicePixelRatio;
    const float halfStripSize = kPixelPerStrip / 2.0f / devicePixelRatio;

    // See waveformrenderersimple.cpp for a detailed explanation of the frame and index calculation
    const int visualFramesSize = dataSize / 2;
    const double firstVisualFrame =
            m_waveformRenderer->getFirstDisplayedPosition(positionType) * visualFramesSize;
    const double lastVisualFrame =
            m_waveformRenderer->getLastDisplayedPosition(positionType) * visualFramesSize;

    // Represents the # of visual frames per horizontal pixel.
    const double visualIncrementPerPixel =
            (lastVisualFrame - firstVisualFrame) / static_cast<double>(stripLength);

    // Per-band gain from the EQ knobs.
    float allGain(1.0);
    // applyCompensation = false, as we scale to match filtered.all
    getGains(&allGain, false, nullptr, nullptr, nullptr);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth());
    const float stemBreadth = m_splitStemTracks ? breadth / 4.0f : 0;
    const float halfBreadth = (m_splitStemTracks ? stemBreadth : breadth) / 2.0f;

    const float heightFactor = allGain * halfBreadth / m_maxValue;

    // Effective visual frame for x
    double xVisualFrame = qRound(firstVisualFrame / visualIncrementPerPixel) *
            visualIncrementPerPixel;

    const int numVerticesPerLine = 6; // 2 triangles

    const int reserved = numVerticesPerLine *
            (mixxx::audio::ChannelCount::stem() * stripLength + 1);

    geometry().setDrawingMode(Geometry::DrawingMode::Triangles);
    geometry().allocate(reserved);
    markDirtyGeometry();

    RGBAVertexUpdater vertexUpdater{geometry().vertexDataAs<Geometry::RGBAColoredPoint2D>()};
    vertexUpdater.addRectangle({0.f,
                                       halfBreadth - 0.5f},
            {static_cast<float>(length),
                    m_isSlipRenderer ? halfBreadth : halfBreadth + 0.5f},
            {0.f, 0.f, 0.f, 0.f});

    const double maxSamplingRange = visualIncrementPerPixel / 2.0;

    for (int visualIdx = 0; visualIdx < stripLength; visualIdx++) {
        for (int stemIdx = 0; stemIdx < mixxx::kMaxSupportedStems; stemIdx++) {
            // Stem is drawn twice with different opacity level, this allow to
            // see the maximum signal by transparency
            for (int layerIdx = 0; layerIdx < 2; layerIdx++) {
                QColor stemColor = stemInfo[stemIdx].getColor();
                float color_r = stemColor.redF(),
                      color_g = stemColor.greenF(),
                      color_b = stemColor.blueF(),
                      color_a = stemColor.alphaF() * (layerIdx ? 0.75f : 0.15f);
                const int visualFrameStart = std::lround(xVisualFrame - maxSamplingRange);
                const int visualFrameStop = std::lround(xVisualFrame + maxSamplingRange);

                const int visualIndexStart = std::max(visualFrameStart * 2, 0);
                const int visualIndexStop =
                        std::min(std::max(visualFrameStop, visualFrameStart + 1) * 2, dataSize - 1);

                const float fVisualIdx = static_cast<float>(visualIdx) * invDevicePixelRatio;

                // Find the max values for current eq in the waveform data.
                // - Max of left and right
                uchar u8max{};
                for (int chn = 0; chn < 2; chn++) {
                    // data is interleaved left / right
                    for (int i = visualIndexStart + chn; i < visualIndexStop + chn; i += 2) {
                        const WaveformData& waveformData = data[i];

                        u8max = math_max(u8max, waveformData.stems[stemIdx]);
                    }
                }

                // Cast to float
                float max = static_cast<float>(u8max);

                // Apply the gains
                if (layerIdx) {
                    bool isMuted = m_pStemMute.empty() ? false : m_pStemMute[stemIdx]->toBool();
                    float volume = m_pStemGain.empty()
                            ? 1.f
                            : static_cast<float>(m_pStemGain[stemIdx]->get());
                    max *= isMuted ||
                                    (selectedStems &&
                                            !(selectedStems & 1 << stemIdx))
                            ? 0.f
                            : volume;
                }

                // Lines are thin rectangles
                // shadow
                vertexUpdater.addRectangle(
                        {fVisualIdx - halfStripSize,
                                stemIdx * stemBreadth + halfBreadth -
                                        heightFactor * max},
                        {fVisualIdx + halfStripSize,
                                m_isSlipRenderer
                                        ? stemIdx * stemBreadth + halfBreadth
                                        : stemIdx * stemBreadth + halfBreadth +
                                                heightFactor * max},
                        {color_r, color_g, color_b, color_a});
            }
        }

        xVisualFrame += visualIncrementPerPixel;
    }

    DEBUG_ASSERT(reserved == vertexUpdater.index());

    markDirtyMaterial();

    return true;
}

} // namespace allshader
