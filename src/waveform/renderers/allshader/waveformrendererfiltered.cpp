#include "waveform/renderers/allshader/waveformrendererfiltered.h"

#include "track/track.h"
#include "util/math.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"

namespace allshader {

WaveformRendererFiltered::WaveformRendererFiltered(
        WaveformWidgetRenderer* waveformWidget, bool bRgbStacked)
        : WaveformRendererSignalBase(waveformWidget),
          m_bRgbStacked(bRgbStacked) {
}

void WaveformRendererFiltered::onSetup(const QDomNode& node) {
    Q_UNUSED(node);
}

void WaveformRendererFiltered::initializeGL() {
    WaveformRendererSignalBase::initializeGL();
    m_shader.init();
}

void WaveformRendererFiltered::paintGL() {
    TrackPointer pTrack = m_waveformRenderer->getTrackInfo();
    if (!pTrack) {
        return;
    }

    ConstWaveformPointer waveform = pTrack->getWaveform();
    if (waveform.isNull()) {
        return;
    }

    const int dataSize = waveform->getDataSize();
    if (dataSize <= 1) {
        return;
    }

    const WaveformData* data = waveform->data();
    if (data == nullptr) {
        return;
    }

    const float devicePixelRatio = m_waveformRenderer->getDevicePixelRatio();
    const int length = static_cast<int>(m_waveformRenderer->getLength() * devicePixelRatio);

    // See waveformrenderersimple.cpp for a detailed explanation of the frame and index calculation
    const int visualFramesSize = dataSize / 2;
    const double firstVisualFrame =
            m_waveformRenderer->getFirstDisplayedPosition() * visualFramesSize;
    const double lastVisualFrame =
            m_waveformRenderer->getLastDisplayedPosition() * visualFramesSize;

    // Represents the # of visual frames per horizontal pixel.
    const double visualIncrementPerPixel =
            (lastVisualFrame - firstVisualFrame) / static_cast<double>(length);

    // Per-band gain from the EQ knobs.
    float allGain{1.0};
    float bandGain[3] = {1.0, 1.0, 1.0};
    getGains(&allGain, true, &bandGain[0], &bandGain[1], &bandGain[2]);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth()) * devicePixelRatio;
    const float halfBreadth = breadth / 2.0f;

    const float heightFactor = allGain * halfBreadth / m_maxValue;

    // Effective visual frame for x
    double xVisualFrame = qRound(firstVisualFrame / visualIncrementPerPixel) *
            visualIncrementPerPixel;

    const int numVerticesPerLine = 6; // 2 triangles

    int reserved[4];
    // low, mid, high
    for (int bandIndex = 0; bandIndex < 3; bandIndex++) {
        m_vertices[bandIndex].clear();
        reserved[bandIndex] = numVerticesPerLine * length;
        m_vertices[bandIndex].reserve(reserved[bandIndex]);
    }

    // the horizontal line
    reserved[3] = numVerticesPerLine;
    m_vertices[3].clear();
    m_vertices[3].reserve(reserved[3]);

    m_vertices[3].addRectangle(
            0.f,
            halfBreadth - 0.5f * devicePixelRatio,
            static_cast<float>(length),
            halfBreadth + 0.5f * devicePixelRatio);

    const double maxSamplingRange = visualIncrementPerPixel / 2.0;

    for (int pos = 0; pos < length; ++pos) {
        const int visualFrameStart = std::lround(xVisualFrame - maxSamplingRange);
        const int visualFrameStop = std::lround(xVisualFrame + maxSamplingRange);

        const int visualIndexStart = std::max(visualFrameStart * 2, 0);
        const int visualIndexStop =
                std::min(std::max(visualFrameStop, visualFrameStart + 1) * 2, dataSize - 1);

        const float fpos = static_cast<float>(pos);

        // 3 bands, 2 channels
        float max[3][2]{};

        for (int i = visualIndexStart; i < visualIndexStop; i += 2) {
            for (int chn = 0; chn < 2; chn++) {
                const WaveformData& waveformData = data[i + chn];
                const float filteredLow = static_cast<float>(waveformData.filtered.low);
                const float filteredMid = static_cast<float>(waveformData.filtered.mid);
                const float filteredHigh = static_cast<float>(waveformData.filtered.high);

                max[0][chn] = math_max(max[0][chn], filteredLow);
                max[1][chn] = math_max(max[1][chn], filteredMid);
                max[2][chn] = math_max(max[2][chn], filteredHigh);
            }
        }

        for (int bandIndex = 0; bandIndex < 3; bandIndex++) {
            max[bandIndex][0] *= bandGain[bandIndex];
            max[bandIndex][1] *= bandGain[bandIndex];

            // lines are thin rectangles
            m_vertices[bandIndex].addRectangle(
                    fpos - 0.5f,
                    halfBreadth - heightFactor * max[bandIndex][0],
                    fpos + 0.5f,
                    halfBreadth + heightFactor * max[bandIndex][1]);
        }

        xVisualFrame += visualIncrementPerPixel;
    }

    const QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, true);

    const int matrixLocation = m_shader.matrixLocation();
    const int colorLocation = m_shader.colorLocation();
    const int positionLocation = m_shader.positionLocation();

    m_shader.bind();
    m_shader.enableAttributeArray(positionLocation);

    m_shader.setUniformValue(matrixLocation, matrix);

    QColor colors[4];
    if (m_bRgbStacked) {
        colors[0].setRgbF(static_cast<float>(m_rgbLowColor_r),
                static_cast<float>(m_rgbLowColor_g),
                static_cast<float>(m_rgbLowColor_b));
        colors[1].setRgbF(static_cast<float>(m_rgbMidColor_r),
                static_cast<float>(m_rgbMidColor_g),
                static_cast<float>(m_rgbMidColor_b));
        colors[2].setRgbF(static_cast<float>(m_rgbHighColor_r),
                static_cast<float>(m_rgbHighColor_g),
                static_cast<float>(m_rgbHighColor_b));
    } else {
        colors[0].setRgbF(static_cast<float>(m_lowColor_r),
                static_cast<float>(m_lowColor_g),
                static_cast<float>(m_lowColor_b));
        colors[1].setRgbF(static_cast<float>(m_midColor_r),
                static_cast<float>(m_midColor_g),
                static_cast<float>(m_midColor_b));
        colors[2].setRgbF(static_cast<float>(m_highColor_r),
                static_cast<float>(m_highColor_g),
                static_cast<float>(m_highColor_b));
    }
    colors[3].setRgbF(static_cast<float>(m_axesColor_r),
            static_cast<float>(m_axesColor_g),
            static_cast<float>(m_axesColor_b),
            static_cast<float>(m_axesColor_a));

    // 3 bands + 1 extra for the horizontal line

    for (int i = 0; i < 4; i++) {
        DEBUG_ASSERT(reserved[i] == m_vertices[i].size());
        m_shader.setUniformValue(colorLocation, colors[i]);
        m_shader.setAttributeArray(
                positionLocation, GL_FLOAT, m_vertices[i].constData(), 2);

        glDrawArrays(GL_TRIANGLES, 0, m_vertices[i].size());
    }

    m_shader.disableAttributeArray(positionLocation);
    m_shader.release();
}

} // namespace allshader
