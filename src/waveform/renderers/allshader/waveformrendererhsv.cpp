#include "waveform/renderers/allshader/waveformrendererhsv.h"

#include "rendergraph/material/rgbmaterial.h"
#include "rendergraph/vertexupdaters/rgbvertexupdater.h"
#include "track/track.h"
#include "util/colorcomponents.h"
#include "util/math.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"

using namespace rendergraph;

namespace allshader {

WaveformRendererHSV::WaveformRendererHSV(WaveformWidgetRenderer* waveformWidget)
        : WaveformRendererSignalBase(waveformWidget) {
    initForRectangles<RGBMaterial>(0);
    setUsePreprocess(true);
}

void WaveformRendererHSV::onSetup(const QDomNode&) {
}

void WaveformRendererHSV::preprocess() {
    if (!preprocessInner()) {
        if (geometry().vertexCount() != 0) {
            geometry().allocate(0);
            markDirtyGeometry();
        }
    }
}

bool WaveformRendererHSV::preprocessInner() {
    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();

    if (!pTrack) {
        return false;
    }

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
    if (!stemInfo.isEmpty() && waveform->hasStem()) {
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
            m_waveformRenderer->getFirstDisplayedPosition() * visualFramesSize;
    const double lastVisualFrame =
            m_waveformRenderer->getLastDisplayedPosition() * visualFramesSize;

    // Represents the # of visual frames per horizontal pixel.
    const double visualIncrementPerPixel =
            (lastVisualFrame - firstVisualFrame) / static_cast<double>(pixelLength);

    float allGain = 1.0f;
    float lowGain = 1.0f;
    float midGain = 1.0f;
    float highGain = 1.0f;
    getGains(&allGain, &lowGain, &midGain, &highGain);

    // Get base color of waveform in the HSV format (s and v isn't use)
    float h, s, v;
    getHsvF(m_waveformRenderer->getWaveformSignalColors()->getLowColor(), &h, &s, &v);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth());
    const float halfBreadth = breadth / 2.0f;

    const float heightFactor = allGain * halfBreadth / m_maxValue;

    // Effective visual frame for x
    double xVisualFrame = qRound(firstVisualFrame / visualIncrementPerPixel) *
            visualIncrementPerPixel;

    const int numVerticesPerLine = 6; // 2 triangles

    const int reserved = numVerticesPerLine * (pixelLength + 1);

    geometry().setDrawingMode(Geometry::DrawingMode::Triangles);
    geometry().allocate(reserved);
    markDirtyGeometry();

    RGBVertexUpdater vertexUpdater{geometry().vertexDataAs<Geometry::RGBColoredPoint2D>()};
    vertexUpdater.addRectangle({0.f,
                                       halfBreadth - 0.5f},
            {static_cast<float>(length),
                    halfBreadth + 0.5f},
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

        // per channel
        float maxLow[2]{};
        float maxMid[2]{};
        float maxHigh[2]{};
        float maxAll[2]{};
        float eqGain[2] = {1.0f, 1.0f};

        for (int chn = 0; chn < 2; chn++) {
            // Find the max values for low, mid, high and all in the waveform data
            float maxLowU = 0.0f;
            float maxMidU = 0.0f;
            float maxHighU = 0.0f;
            float maxAllU = 0.0f;
            // data is interleaved left / right
            for (int i = visualIndexStart + chn; i < visualIndexStop + chn; i += 2) {
                const WaveformData& waveformData = data[i];

                maxLowU = math_max(maxLowU,
                        static_cast<float>(
                                waveformData.filtered.low));
                maxMidU = math_max(maxMidU,
                        static_cast<float>(
                                waveformData.filtered.mid));
                maxHighU = math_max(maxHighU,
                        static_cast<float>(
                                waveformData.filtered.high));
                maxAllU = math_max(maxAllU, static_cast<float>(waveformData.filtered.all));
            }

            maxLow[chn] = maxLowU * lowGain;
            maxMid[chn] = maxMidU * midGain;
            maxHigh[chn] = maxHighU * highGain;
            maxAll[chn] = maxAllU;

            float allUnscaled = maxLowU + maxMidU + maxHighU;
            if (allUnscaled > 0.0f) {
                eqGain[chn] = (maxLow[chn] + maxMid[chn] + maxHigh[chn]) / allUnscaled;
            }
        }

        float total{};
        float lo{};
        float hi{};

        if (maxAll[0] != 0.f && maxAll[1] != 0.f) {
            // Calculate sum, to normalize
            // Also multiply on 1.2 to prevent very dark or light color
            total = (maxLow[0] + maxLow[1] + maxMid[0] + maxMid[1] +
                            maxHigh[0] + maxHigh[1]) *
                    1.2f;

            // prevent division by zero
            if (total != 0.f) {
                // Normalize low and high (mid not need, because it not change the color)
                lo = (maxLow[0] + maxLow[1]) / total;
                hi = (maxHigh[0] + maxHigh[1]) / total;
            }
        }

        // Set color
        QColor color;
        color.setHsvF(h, 1.0f - hi, 1.0f - lo);

        // Lines are thin rectangles
        // maxAll[0] is for left channel, maxAll[1] is for right channel
        vertexUpdater.addRectangle({fpos - halfPixelSize,
                                           halfBreadth - heightFactor * eqGain[0] * maxAll[0]},
                {fpos + halfPixelSize,
                        halfBreadth + heightFactor * eqGain[1] * maxAll[1]},
                {static_cast<float>(color.redF()),
                        static_cast<float>(color.greenF()),
                        static_cast<float>(color.blueF())});

        xVisualFrame += visualIncrementPerPixel;
    }

    DEBUG_ASSERT(reserved == vertexUpdater.index());

    markDirtyMaterial();

    return true;
}

} // namespace allshader
