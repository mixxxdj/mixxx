#include "waveform/renderers/allshader/waveformrendererrgb3band.h"

#include <algorithm>
#include <cmath>

#include "rendergraph/material/rgbmaterial.h"
#include "rendergraph/vertexupdaters/rgbvertexupdater.h"
#include "track/track.h"
#include "util/colorcomponents.h"
#include "waveform/renderers/waveformsignalcolors.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"

using namespace rendergraph;

namespace {

// Each band is a peak envelope with instant attack and exponential release.
// The constants were fitted against Rekordbox's 3-band analysis data. The low
// band has the longest release, which also determines the lead-in.
constexpr double kReleaseSeconds[3] = {0.2, 0.1, 0.1};
// Envelopes decay below 1/256 after this many release time constants.
constexpr double kLeadInReleases = 5.6;
// Height relative to half the breadth for a full-scale envelope.
constexpr float kBandGain[3] = {0.77f, 1.23f, 0.44f};
constexpr float kHighExponent = 1.25f;

// Layers in drawing order
constexpr int kLowLayer = 0;
constexpr int kMidLayer = 1;
constexpr int kLowMidLayer = 2;
constexpr int kHighLayer = 3;
constexpr int kLayerCount = 4;
constexpr int kBandLayer[3] = {kLowLayer, kMidLayer, kHighLayer};

} // namespace

namespace allshader {

WaveformRendererRGB3Band::WaveformRendererRGB3Band(
        WaveformWidgetRenderer* waveformWidget,
        ::WaveformRendererSignalBase::Options options)
        : WaveformRendererSignalBase(waveformWidget, options),
          m_lowMidColor_r(0),
          m_lowMidColor_g(0),
          m_lowMidColor_b(0) {
    initForRectangles<RGBMaterial>(0);
    setUsePreprocess(true);
}

void WaveformRendererRGB3Band::onSetup(const QDomNode&) {
    // Skins configure these separately from the Filtered colors
    const auto* pColors = m_waveformRenderer->getWaveformSignalColors();
    getRgbF(pColors->getRgb3BandLowColor(), &m_lowColor_r, &m_lowColor_g, &m_lowColor_b);
    getRgbF(pColors->getRgb3BandMidColor(), &m_midColor_r, &m_midColor_g, &m_midColor_b);
    getRgbF(pColors->getRgb3BandHighColor(), &m_highColor_r, &m_highColor_g, &m_highColor_b);
    setLowMidColor(pColors->getRgb3BandLowMidColor());
}

void WaveformRendererRGB3Band::setLowMidColor(const QColor& lowMidColor) {
    getRgbF(lowMidColor, &m_lowMidColor_r, &m_lowMidColor_g, &m_lowMidColor_b);
}

void WaveformRendererRGB3Band::preprocess() {
    if (!preprocessInner()) {
        if (geometry().vertexCount() != 0) {
            geometry().allocate(0);
            markDirtyGeometry();
        }
    }
}

// Calculates the layer heights in pixels for every pixel. The envelopes run
// over the stereo-combined visual frames, starting early enough to be settled
// at the first pixel, so scrolling does not change the shape.
void WaveformRendererRGB3Band::calculateHeights(const WaveformData* data,
        int visualFramesSize,
        double visualSampleRate,
        double firstPixelVisualFrame,
        double visualIncrementPerPixel,
        int pixelLength,
        const float bandScale[3],
        float maxHeight) {
    const double halfPixelFrames = visualIncrementPerPixel / 2.0;
    const int leadInFrames = static_cast<int>(
            std::ceil(kLeadInReleases * kReleaseSeconds[0] * visualSampleRate));
    const int firstFrame = static_cast<int>(std::floor(firstPixelVisualFrame - halfPixelFrames)) -
            leadInFrames;
    const int lastFrame = static_cast<int>(std::ceil(firstPixelVisualFrame +
                                  (pixelLength - 1) * visualIncrementPerPixel + halfPixelFrames)) +
            1;
    const int frameCount = lastFrame - firstFrame + 1;

    float decay[3];
    for (int band = 0; band < 3; ++band) {
        decay[band] = static_cast<float>(
                std::exp(-1.0 / (kReleaseSeconds[band] * visualSampleRate)));
        m_envelopes[band].resize(frameCount);
    }

    // Stereo-combined amplitudes: low, mid, and mid + high. Mid and high
    // together match Rekordbox's high band better than high alone.
    constexpr float kInvMaxSquared = 1.0f / (255.0f * 255.0f);
    float envelope[3]{};
    for (int i = 0; i < frameCount; ++i) {
        const int frame = firstFrame + i;
        float squared[3]{};
        if (frame >= 0 && frame < visualFramesSize) {
            for (int chn = 0; chn < 2; ++chn) {
                const WaveformFilteredData& filtered = data[frame * 2 + chn].filtered;
                const float mid = static_cast<float>(filtered.mid) * filtered.mid;
                squared[0] += static_cast<float>(filtered.low) * filtered.low;
                squared[1] += mid;
                squared[2] += mid + static_cast<float>(filtered.high) * filtered.high;
            }
        }
        for (int band = 0; band < 3; ++band) {
            const float amplitude = std::sqrt(0.5f * squared[band] * kInvMaxSquared);
            envelope[band] = std::max(amplitude, envelope[band] * decay[band]);
            m_envelopes[band][i] = envelope[band];
        }
    }

    for (int layer = 0; layer < kLayerCount; ++layer) {
        m_heights[layer].resize(pixelLength);
    }

    for (int pos = 0; pos < pixelLength; ++pos) {
        const double center = firstPixelVisualFrame + pos * visualIncrementPerPixel - firstFrame;
        for (int band = 0; band < 3; ++band) {
            const std::vector<float>& bandEnvelope = m_envelopes[band];
            float value;
            if (visualIncrementPerPixel > 1.0) {
                // Zoomed out: peak over the frames covered by this pixel.
                const int start = static_cast<int>(std::lround(center - halfPixelFrames));
                const int stop = std::max(start + 1,
                        static_cast<int>(std::lround(center + halfPixelFrames)));
                value = *std::max_element(bandEnvelope.begin() + start,
                        bandEnvelope.begin() + stop);
            } else {
                // Zoomed in: interpolate between frames.
                const int index = static_cast<int>(center);
                const float fraction = static_cast<float>(center - index);
                value = bandEnvelope[index] +
                        fraction * (bandEnvelope[index + 1] - bandEnvelope[index]);
            }
            if (kBandLayer[band] == kHighLayer) {
                value = std::pow(value, kHighExponent);
            }
            m_heights[kBandLayer[band]][pos] =
                    std::min(maxHeight, kBandGain[band] * bandScale[band] * value);
        }
        m_heights[kLowMidLayer][pos] =
                std::min(m_heights[kLowLayer][pos], m_heights[kMidLayer][pos]);
    }
}

bool WaveformRendererRGB3Band::preprocessInner() {
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
    if (!stemInfo.isEmpty() && waveform->hasStem() && !m_ignoreStem) {
        return false;
    }
#endif

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    const int length = static_cast<int>(m_waveformRenderer->getLength());
    const int pixelLength = static_cast<int>(m_waveformRenderer->getLength() * devicePixelRatio);
    const float invDevicePixelRatio = 1.f / devicePixelRatio;

    // See waveformrenderersimple.cpp for a detailed explanation of the frame and index calculation
    const int visualFramesSize = dataSize / 2;
    const double firstVisualFrame =
            m_waveformRenderer->getFirstDisplayedPosition() * visualFramesSize;
    const double lastVisualFrame =
            m_waveformRenderer->getLastDisplayedPosition() * visualFramesSize;

    // Represents the # of visual frames per horizontal pixel.
    const double visualIncrementPerPixel =
            (lastVisualFrame - firstVisualFrame) / static_cast<double>(pixelLength);

    // Per-band gain from the EQ knobs.
    float allGain(1.0);
    float bandGain[3] = {1.0, 1.0, 1.0};
    getGains(&allGain, &bandGain[0], &bandGain[1], &bandGain[2]);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth());
    const float halfBreadth = breadth / 2.0f;

    // Effective visual frame of the first pixel
    const double firstPixelVisualFrame =
            qRound(firstVisualFrame / visualIncrementPerPixel) * visualIncrementPerPixel;
    const double visualSampleRate =
            pTrack->getSampleRate().toDouble() / waveform->getAudioVisualRatio();
    const float bandScale[3] = {allGain * halfBreadth * bandGain[0],
            allGain * halfBreadth * bandGain[1],
            allGain * halfBreadth * bandGain[2]};
    calculateHeights(data,
            visualFramesSize,
            visualSampleRate,
            firstPixelVisualFrame,
            visualIncrementPerPixel,
            pixelLength,
            bandScale,
            halfBreadth);

    const int numVerticesPerLine = 6; // 2 triangles

    // Connected segments between neighboring pixels for each layer + horizontal axis
    const int segmentCount = std::max(0, pixelLength - 1);
    const int reserved = numVerticesPerLine * (segmentCount * kLayerCount + 1);

    geometry().setDrawingMode(Geometry::DrawingMode::Triangles);
    geometry().allocate(reserved);
    markDirtyGeometry();

    RGBVertexUpdater vertexUpdater{geometry().vertexDataAs<Geometry::RGBColoredPoint2D>()};
    vertexUpdater.addRectangle({0.f, halfBreadth - 0.5f},
            {static_cast<float>(length), halfBreadth + 0.5f},
            {static_cast<float>(m_axesColor_r),
                    static_cast<float>(m_axesColor_g),
                    static_cast<float>(m_axesColor_b)});

    QVector3D rgb[kLayerCount];
    rgb[kLowLayer] = QVector3D(static_cast<float>(m_lowColor_r),
            static_cast<float>(m_lowColor_g),
            static_cast<float>(m_lowColor_b));
    rgb[kMidLayer] = QVector3D(static_cast<float>(m_midColor_r),
            static_cast<float>(m_midColor_g),
            static_cast<float>(m_midColor_b));
    rgb[kLowMidLayer] = QVector3D(m_lowMidColor_r, m_lowMidColor_g, m_lowMidColor_b);
    rgb[kHighLayer] = QVector3D(static_cast<float>(m_highColor_r),
            static_cast<float>(m_highColor_g),
            static_cast<float>(m_highColor_b));

    // Mirrored trapezoids between neighboring pixels
    for (int layer = 0; layer < kLayerCount; ++layer) {
        const std::vector<float>& height = m_heights[layer];
        for (int pos = 0; pos < segmentCount; ++pos) {
            const float x1 = static_cast<float>(pos) * invDevicePixelRatio;
            const float x2 = static_cast<float>(pos + 1) * invDevicePixelRatio;
            const float top1 = halfBreadth - height[pos];
            const float top2 = halfBreadth - height[pos + 1];
            const float bottom1 = halfBreadth + height[pos];
            const float bottom2 = halfBreadth + height[pos + 1];
            vertexUpdater.addTriangle({x1, top1}, {x2, top2}, {x1, bottom1}, rgb[layer]);
            vertexUpdater.addTriangle({x2, top2}, {x2, bottom2}, {x1, bottom1}, rgb[layer]);
        }
    }

    DEBUG_ASSERT(reserved == vertexUpdater.index());

    markDirtyMaterial();

    return true;
}

} // namespace allshader
