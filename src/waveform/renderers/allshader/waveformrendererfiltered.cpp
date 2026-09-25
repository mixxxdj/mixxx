#include "waveform/renderers/allshader/waveformrendererfiltered.h"

#include <algorithm>

#include "rendergraph/material/rgbmaterial.h"
#include "rendergraph/vertexupdaters/rgbvertexupdater.h"
#include "track/track.h"
#include "util/math.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"
#include "waveform/waveformanalysisprofile.h"

using namespace rendergraph;

namespace allshader {

WaveformRendererFiltered::WaveformRendererFiltered(
        WaveformWidgetRenderer* waveformWidget,
        Mode mode,
        ::WaveformRendererSignalBase::Options options)
        : WaveformRendererSignalBase(waveformWidget, options),
          m_mode(mode) {
    initForRectangles<RGBMaterial>(0);
    setUsePreprocess(true);
}

void WaveformRendererFiltered::onSetup(const QDomNode&) {
}

void WaveformRendererFiltered::preprocess() {
    if (!preprocessInner()) {
        if (geometry().vertexCount() != 0) {
            geometry().allocate(0);
            markDirtyGeometry();
        }
    }
}

bool WaveformRendererFiltered::preprocessInner() {
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

    // Per-band gain from the EQ knobs.
    float allGain(1.0);
    float bandGain[3] = {1.0, 1.0, 1.0};
    getGains(&allGain, &bandGain[0], &bandGain[1], &bandGain[2]);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth());
    const float halfBreadth = breadth / 2.0f;
    const bool useRgbColors = m_mode != Mode::Filtered;
    const bool usePerceptualThreeBand = m_mode == Mode::Perceptual3Band;
    const int layerCount = usePerceptualThreeBand ? 4 : 3;

    const float maximumValue = usePerceptualThreeBand
            ? mixxx::kPerceptual3BandMaximumValue
            : m_maxValue;
    const float heightFactor = allGain * halfBreadth / maximumValue;

    // Effective visual frame for x
    double xVisualFrame = qRound(firstVisualFrame / visualIncrementPerPixel) *
            visualIncrementPerPixel;

    const int numVerticesPerLine = 6; // 2 triangles

    int reserved = numVerticesPerLine * (pixelLength * layerCount + 1);

    geometry().setDrawingMode(Geometry::DrawingMode::Triangles);
    geometry().allocate(reserved);
    markDirtyGeometry();

    QVector3D rgb[4];
    if (usePerceptualThreeBand) {
        rgb[0] = QVector3D(32.0f / 255.0f, 83.0f / 255.0f, 217.0f / 255.0f);
        rgb[1] = QVector3D(242.0f / 255.0f, 170.0f / 255.0f, 60.0f / 255.0f);
        rgb[2] = QVector3D(169.0f / 255.0f, 107.0f / 255.0f, 39.0f / 255.0f);
        rgb[3] = QVector3D(1.0f, 1.0f, 1.0f);
    } else if (useRgbColors) {
        rgb[0] = QVector3D(static_cast<float>(m_rgbLowColor_r),
                static_cast<float>(m_rgbLowColor_g),
                static_cast<float>(m_rgbLowColor_b));
        rgb[1] = QVector3D(static_cast<float>(m_rgbMidColor_r),
                static_cast<float>(m_rgbMidColor_g),
                static_cast<float>(m_rgbMidColor_b));
        rgb[2] = QVector3D(static_cast<float>(m_rgbHighColor_r),
                static_cast<float>(m_rgbHighColor_g),
                static_cast<float>(m_rgbHighColor_b));
    } else {
        rgb[0] = QVector3D(static_cast<float>(m_lowColor_r),
                static_cast<float>(m_lowColor_g),
                static_cast<float>(m_lowColor_b));
        rgb[1] = QVector3D(static_cast<float>(m_midColor_r),
                static_cast<float>(m_midColor_g),
                static_cast<float>(m_midColor_b));
        rgb[2] = QVector3D(static_cast<float>(m_highColor_r),
                static_cast<float>(m_highColor_g),
                static_cast<float>(m_highColor_b));
    }

    RGBVertexUpdater axisVertexUpdater{geometry().vertexDataAs<Geometry::RGBColoredPoint2D>()};
    axisVertexUpdater.addRectangle({0.f,
                                           halfBreadth - 0.5f},
            {static_cast<float>(length),
                    halfBreadth + 0.5f},
            {static_cast<float>(m_axesColor_r),
                    static_cast<float>(m_axesColor_g),
                    static_cast<float>(m_axesColor_b)});

    RGBVertexUpdater vertexUpdater[4]{
            {geometry().vertexDataAs<Geometry::RGBColoredPoint2D>() +
                    numVerticesPerLine},
            {geometry().vertexDataAs<Geometry::RGBColoredPoint2D>() +
                    numVerticesPerLine * (1 + pixelLength)},
            {geometry().vertexDataAs<Geometry::RGBColoredPoint2D>() +
                    numVerticesPerLine * (1 + pixelLength * 2)},
            {geometry().vertexDataAs<Geometry::RGBColoredPoint2D>() +
                    numVerticesPerLine * (1 + pixelLength * 3)}};
    const double maxSamplingRange = visualIncrementPerPixel / 2.0;
    float previousHeight[4][2]{};

    for (int pos = 0; pos < pixelLength; ++pos) {
        const int visualFrameStart = std::lround(xVisualFrame - maxSamplingRange);
        const int visualFrameStop = std::lround(xVisualFrame + maxSamplingRange);

        const int visualIndexStart = std::max(visualFrameStart * 2, 0);
        const int visualIndexStop =
                std::min(std::max(visualFrameStop, visualFrameStart + 1) * 2, dataSize - 1);

        const float fpos = static_cast<float>(pos) * invDevicePixelRatio;

        // 3 bands, 2 channels
        float max[3][2]{};
        uchar u8max[3][2]{};
        for (int chn = 0; chn < 2; chn++) {
            for (int i = visualIndexStart + chn; i < visualIndexStop + chn; i += 2) {
                const WaveformData& waveformData = data[i];

                u8max[0][chn] = math_max(u8max[0][chn], waveformData.filtered.low);
                u8max[1][chn] = math_max(u8max[1][chn], waveformData.filtered.mid);
                u8max[2][chn] = math_max(u8max[2][chn], waveformData.filtered.high);
            }
        }
        for (int chn = 0; chn < 2; ++chn) {
            max[0][chn] = static_cast<float>(u8max[0][chn]);
            max[1][chn] = static_cast<float>(u8max[1][chn]);
            max[2][chn] = static_cast<float>(u8max[2][chn]);
        }

        // TODO: this can be optimized by using one geometrynode per band
        // + one for the horizontal axis, and uniform color materials,
        // instead of passing constant color as vertex.

        for (int bandIndex = 0; bandIndex < 3; bandIndex++) {
            max[bandIndex][0] *= bandGain[bandIndex];
            max[bandIndex][1] *= bandGain[bandIndex];
        }

        float height[4][2]{};
        if (usePerceptualThreeBand) {
            for (int channelIndex = 0; channelIndex < 2; channelIndex++) {
                const float lowHeight = heightFactor * max[0][channelIndex];
                const float midHeight = heightFactor * max[1][channelIndex];
                height[0][channelIndex] = lowHeight > midHeight ? lowHeight : 0.0f;
                height[1][channelIndex] = midHeight > lowHeight ? midHeight : 0.0f;
                height[2][channelIndex] = std::min(lowHeight, midHeight);
                height[3][channelIndex] = heightFactor * max[2][channelIndex];
            }
        } else {
            for (int bandIndex = 0; bandIndex < 3; bandIndex++) {
                height[bandIndex][0] = heightFactor * max[bandIndex][0];
                height[bandIndex][1] = heightFactor * max[bandIndex][1];
            }
        }

        for (int layerIndex = 0; layerIndex < layerCount; layerIndex++) {
            if (usePerceptualThreeBand && pos > 0) {
                const float x0 = fpos - halfPixelSize;
                const float x1 = fpos + halfPixelSize;
                const float top0 = halfBreadth - previousHeight[layerIndex][0];
                const float top1 = halfBreadth - height[layerIndex][0];
                const float bottom0 = halfBreadth + previousHeight[layerIndex][1];
                const float bottom1 = halfBreadth + height[layerIndex][1];
                vertexUpdater[layerIndex].addTriangle(
                        {x0, top0}, {x1, top1}, {x0, bottom0}, rgb[layerIndex]);
                vertexUpdater[layerIndex].addTriangle(
                        {x0, bottom0}, {x1, bottom1}, {x1, top1}, rgb[layerIndex]);
            } else {
                vertexUpdater[layerIndex].addRectangle(
                        {fpos - halfPixelSize, halfBreadth - height[layerIndex][0]},
                        {fpos + halfPixelSize, halfBreadth + height[layerIndex][1]},
                        {rgb[layerIndex]});
            }
            previousHeight[layerIndex][0] = height[layerIndex][0];
            previousHeight[layerIndex][1] = height[layerIndex][1];
        }

        xVisualFrame += visualIncrementPerPixel;
    }

    int writtenVertices = numVerticesPerLine;
    for (int layerIndex = 0; layerIndex < layerCount; layerIndex++) {
        writtenVertices += vertexUpdater[layerIndex].index();
    }
    DEBUG_ASSERT(reserved == writtenVertices);

    markDirtyMaterial();

    return true;
}

} // namespace allshader
