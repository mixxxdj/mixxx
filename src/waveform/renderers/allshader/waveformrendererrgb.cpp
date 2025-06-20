#include "waveform/renderers/allshader/waveformrendererrgb.h"

#include "rendergraph/material/rgbmaterial.h"
#include "rendergraph/vertexupdaters/rgbvertexupdater.h"
#include "track/track.h"
#include "util/colorcomponents.h"
#include "util/math.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"

using namespace rendergraph;

namespace allshader {

namespace {
inline float math_pow2(float x) {
    return x * x;
}
} // namespace

WaveformRendererRGB::WaveformRendererRGB(WaveformWidgetRenderer* waveformWidget,
        ::WaveformRendererAbstract::PositionSource type,
        WaveformRendererSignalBase::Options options)
        : WaveformRendererSignalBase(waveformWidget),
          m_isSlipRenderer(type == ::WaveformRendererAbstract::Slip),
          m_options(options) {
    initForRectangles<RGBMaterial>(0);
    setUsePreprocess(true);
}

void WaveformRendererRGB::onSetup(const QDomNode&) {
}

void WaveformRendererRGB::preprocess() {
    if (!preprocessInner()) {
        if (geometry().vertexCount() != 0) {
            geometry().allocate(0);
            markDirtyGeometry();
        }
    }
}

bool WaveformRendererRGB::preprocessInner() {
    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();

    if (!pTrack || (m_isSlipRenderer && !m_waveformRenderer->isSlipActive())) {
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
#ifdef __STEM__
    auto stemInfo = pTrack->getStemInfo();
    // If this track is a stem track, skip the rendering
    if (!stemInfo.isEmpty() && waveform->hasStem() && !m_ignoreStem) {
        return false;
    }
#endif

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    const int length = static_cast<int>(m_waveformRenderer->getLength());
    const int pixelLength = static_cast<int>(m_waveformRenderer->getLength() * devicePixelRatio);
    const float invDevicePixelRatio = 1.f / devicePixelRatio;
    const float halfPixelSize = 0.5f / devicePixelRatio;

    // See waveformrenderersimple.cpp for a detailed explanation of the frame and index calculation
    const int visualFramesSize = dataSize / 2;
    const double firstVisualFrame =
            m_waveformRenderer->getFirstDisplayedPosition(positionType) * visualFramesSize;
    const double lastVisualFrame =
            m_waveformRenderer->getLastDisplayedPosition(positionType) * visualFramesSize;

    // Represents the # of visual frames per horizontal pixel.
    const double visualIncrementPerPixel =
            (lastVisualFrame - firstVisualFrame) / static_cast<double>(pixelLength);

    // Per-band gain from the EQ knobs.
    float allGain(1.0), lowGain(1.0), midGain(1.0), highGain(1.0);
    // applyCompensation = false, as we scale to match filtered.all
    getGains(&allGain, false, &lowGain, &midGain, &highGain);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth());
    const float halfBreadth = breadth / 2.0f;

    const float heightFactorAbs = allGain * halfBreadth / m_maxValue;
    const float heightFactor[2] = {-heightFactorAbs, heightFactorAbs};
    const bool splitLeftRight = m_options & WaveformRendererSignalBase::Option::SplitStereoSignal;

    const float low_r = static_cast<float>(m_rgbLowColor_r);
    const float mid_r = static_cast<float>(m_rgbMidColor_r);
    const float high_r = static_cast<float>(m_rgbHighColor_r);
    const float low_g = static_cast<float>(m_rgbLowColor_g);
    const float mid_g = static_cast<float>(m_rgbMidColor_g);
    const float high_g = static_cast<float>(m_rgbHighColor_g);
    const float low_b = static_cast<float>(m_rgbLowColor_b);
    const float mid_b = static_cast<float>(m_rgbMidColor_b);
    const float high_b = static_cast<float>(m_rgbHighColor_b);

    // Effective visual frame for x
    double xVisualFrame = qRound(firstVisualFrame / visualIncrementPerPixel) *
            visualIncrementPerPixel;

    const int numVerticesPerLine = 6; // 2 triangles

    const int reserved = numVerticesPerLine *
            // Slip renderer only render a single channel, so the vertices count doesn't change
            ((splitLeftRight && !m_isSlipRenderer ? pixelLength * 2 : pixelLength) + 1);

    geometry().setDrawingMode(Geometry::DrawingMode::Triangles);
    geometry().allocate(reserved);
    markDirtyGeometry();

    RGBVertexUpdater vertexUpdater{geometry().vertexDataAs<Geometry::RGBColoredPoint2D>()};
    vertexUpdater.addRectangle({0.f,
                                       halfBreadth - 0.5f},
            {static_cast<float>(length),
                    m_isSlipRenderer ? halfBreadth : halfBreadth + 0.5f},
            {static_cast<float>(m_axesColor_r),
                    static_cast<float>(m_axesColor_g),
                    static_cast<float>(m_axesColor_b)});

    const double maxSamplingRange = visualIncrementPerPixel / 2.0;

    for (int pos = 0; pos < pixelLength; ++pos) {
        const int visualFrameStart = std::lround(xVisualFrame - maxSamplingRange);
        const int visualFrameStop = std::lround(xVisualFrame + maxSamplingRange);

        const int visualIndexStart = std::max(visualFrameStart * 2, 0);
        const int visualIndexStop =
                std::min(std::max(visualFrameStop, visualFrameStart + 1) * 2, dataSize - 1);

        const float fpos = static_cast<float>(pos) * invDevicePixelRatio;

        // Find the max values for low, mid, high and all in the waveform data.
        // - Max of left and right
        uchar u8maxLow[2]{};
        uchar u8maxMid[2]{};
        uchar u8maxHigh[2]{};
        // - Per channel
        uchar u8maxAllChn[2]{};
        for (int chn = 0; chn < 2; chn++) {
            // In case we don't render individual color per channel, we use only
            // the first field of the arrays to perform signal max
            int signalChn = splitLeftRight ? chn : 0;
            // data is interleaved left / right
            for (int i = visualIndexStart + chn; i < visualIndexStop + chn; i += 2) {
                const WaveformData& waveformData = data[i];

                u8maxLow[signalChn] = math_max(u8maxLow[signalChn], waveformData.filtered.low);
                u8maxMid[signalChn] = math_max(u8maxMid[signalChn], waveformData.filtered.mid);
                u8maxHigh[signalChn] = math_max(u8maxHigh[signalChn], waveformData.filtered.high);
                u8maxAllChn[chn] = math_max(u8maxAllChn[chn], waveformData.filtered.all);
            }
        }
        float maxAllChn[2]{static_cast<float>(u8maxAllChn[0]), static_cast<float>(u8maxAllChn[1])};

        // In case we don't render individual color per channel, all the
        // signal information is in the first field of each array. If
        // this is the split render, we only render the left channel
        // anyway.
        for (int chn = 0;
                chn < (splitLeftRight && !m_isSlipRenderer ? 2 : 1);
                chn++) {
            // Cast to float
            float maxLow = static_cast<float>(u8maxLow[chn]);
            float maxMid = static_cast<float>(u8maxMid[chn]);
            float maxHigh = static_cast<float>(u8maxHigh[chn]);

            // Calculate the squared magnitude of the maxLow, maxMid and maxHigh values.
            // We take the square root to get the magnitude below.
            const float sum = math_pow2(maxLow) + math_pow2(maxMid) + math_pow2(maxHigh);

            // Apply the gains
            maxLow *= lowGain;
            maxMid *= midGain;
            maxHigh *= highGain;

            // Calculate the squared magnitude of the gained maxLow, maxMid and maxHigh values
            // We take the square root to get the magnitude below.
            const float sumGained = math_pow2(maxLow) + math_pow2(maxMid) + math_pow2(maxHigh);

            // The maxAll values will be used to draw the amplitude. We scale them according to
            // magnitude of the gained maxLow, maxMid and maxHigh values
            if (sum != 0.f) {
                // magnitude = sqrt(sum) and magnitudeGained = sqrt(sumGained), and
                // factor = magnitudeGained / magnitude, but we can do with a single sqrt:
                const float factor = std::sqrt(sumGained / sum);
                maxAllChn[chn] *= factor;
                if (!splitLeftRight) {
                    maxAllChn[chn + 1] *= factor;
                }
            }

            // Use the gained maxLow, maxMid and maxHigh values to calculate the color components
            float red = maxLow * low_r + maxMid * mid_r + maxHigh * high_r;
            float green = maxLow * low_g + maxMid * mid_g + maxHigh * high_g;
            float blue = maxLow * low_b + maxMid * mid_b + maxHigh * high_b;

            // Normalize the color components using the maximum of the three
            const float maxComponent = math_max3(red, green, blue);
            if (maxComponent == 0.f) {
                // Avoid division by 0
                red = 0.f;
                green = 0.f;
                blue = 0.f;
            } else {
                const float normFactor = 1.f / maxComponent;
                red *= normFactor;
                green *= normFactor;
                blue *= normFactor;
            }

            // Lines are thin rectangles
            if (!splitLeftRight) {
                vertexUpdater.addRectangle({fpos - halfPixelSize,
                                                   halfBreadth - heightFactorAbs * maxAllChn[0]},
                        {fpos + halfPixelSize,
                                m_isSlipRenderer
                                        ? halfBreadth
                                        : halfBreadth + heightFactorAbs * maxAllChn[1]},
                        {red,
                                green,
                                blue});
            } else {
                // note: heightFactor is the same for left and right,
                // but negative for left (chn 0) and positive for right (chn 1)
                vertexUpdater.addRectangle({fpos - halfPixelSize,
                                                   halfBreadth},
                        {fpos + halfPixelSize,
                                halfBreadth + heightFactor[chn] * maxAllChn[chn]},
                        {red,
                                green,
                                blue});
            }
        }

        xVisualFrame += visualIncrementPerPixel;
    }

    DEBUG_ASSERT(reserved == vertexUpdater.index());

    markDirtyMaterial();

    return true;
}

} // namespace allshader
