#include "waveform/renderers/qopengl/waveformrendererfiltered.h"

#include "track/track.h"
#include "util/math.h"
#include "waveform/renderers/qopengl/calculatematrix.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"
#include "waveform/widgets/qopengl/waveformwidget.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

using namespace qopengl;

WaveformRendererFiltered::WaveformRendererFiltered(
        WaveformWidgetRenderer* waveformWidget)
        : WaveformRendererSignalBase(waveformWidget) {
}

WaveformRendererFiltered::~WaveformRendererFiltered() {
}

void WaveformRendererFiltered::onSetup(const QDomNode& node) {
    Q_UNUSED(node);
}

void WaveformRendererFiltered::initializeGL() {
    m_shader.init();
}

void WaveformRendererFiltered::renderGL() {
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

    // Not multiplying with devicePixelRatio will also work. In that case, on
    // High-DPI-Display the lines will be devicePixelRatio pixels wide (which is
    // also what is used for the beat grid and the markers), or in other words
    // each block of samples is represented by devicePixelRatio pixels (width).

    const double firstVisualIndex = m_waveformRenderer->getFirstDisplayedPosition() * dataSize;
    const double lastVisualIndex = m_waveformRenderer->getLastDisplayedPosition() * dataSize;

    // Represents the # of waveform data points per horizontal pixel.
    const double gain = (lastVisualIndex - firstVisualIndex) / static_cast<double>(length);

    // Per-band gain from the EQ knobs.
    float channelGain(1.0),
    float bandGain[3] = {1.0, 1.0, 1.0};
    getGains(&allGain, &bandGain[0], &bandGain[1], &bandGain[2]);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth()) * devicePixelRatio;
    const float halfBreadth = breadth / 2.0f;

    const float heightFactor = allGain * halfBreadth / 255.f;

    // Effective visual index of x
    double xVisualSampleIndex = firstVisualIndex;

    const int numVerticesPerLine = 6; // 2 triangles

    int reserved[4];
    // low, mid, high
    for (int bandIndex = 0; bandIndex < 3; bandIndex++) {
        m_verticesForGroup[bandIndex].clear();
        reserved[bandIndex] = numVerticesPerLine * length;
        m_verticesForGroup[bandIndex].reserve(reserved[bandIndex]);
    }

    // the horizontal line
    reserved[3] = numVerticesPerLine;
    m_verticesForGroup[3].clear();
    m_verticesForGroup[3].reserve(reserved[3]);

    m_verticesForGroup[3].addRectangle(
            0.f,
            halfBreadth - 0.5f * devicePixelRatio,
            static_cast<float>(length),
            halfBreadth + 0.5f * devicePixelRatio);

    for (int pos = 0; pos < length; ++pos) {
        // Our current pixel (x) corresponds to a number of visual samples
        // (visualSamplerPerPixel) in our waveform object. We take the max of
        // all the data points on either side of xVisualSampleIndex within a
        // window of 'maxSamplingRange' visual samples to measure the maximum
        // data point contained by this pixel.
        double maxSamplingRange = gain / 2.0;

        // Since xVisualSampleIndex is in visual-samples (e.g. R,L,R,L) we want
        // to check +/- maxSamplingRange frames, not samples. To do this, divide
        // xVisualSampleIndex by 2. Since frames indices are integers, we round
        // to the nearest integer by adding 0.5 before casting to int.
        int visualFrameStart = int(xVisualSampleIndex / 2.0 - maxSamplingRange + 0.5);
        int visualFrameStop = int(xVisualSampleIndex / 2.0 + maxSamplingRange + 0.5);
        const int lastVisualFrame = dataSize / 2 - 1;

        // We now know that some subset of [visualFrameStart, visualFrameStop]
        // lies within the valid range of visual frames. Clamp
        // visualFrameStart/Stop to within [0, lastVisualFrame].
        visualFrameStart = math_clamp(visualFrameStart, 0, lastVisualFrame);
        visualFrameStop = math_clamp(visualFrameStop, 0, lastVisualFrame);

        int visualIndexStart = visualFrameStart * 2;
        int visualIndexStop = visualFrameStop * 2;

        visualIndexStart = std::max(visualIndexStart, 0);
        visualIndexStop = std::min(visualIndexStop, dataSize);

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

        const float fpos = static_cast<float>(pos);

        for (int bandIndex = 0; bandIndex < 3; bandIndex++) {
            max[bandIndex][0] *= bandGain[bandIndex];
            max[bandIndex][1] *= bandGain[bandIndex];

            // lines are thin rectangles
            m_verticesForGroup[bandIndex].addRectangle(
                    fpos - 0.5f,
                    halfBreadth - heightFactor * max[bandIndex][0],
                    fpos + 0.5f,
                    halfBreadth + heightFactor * max[bandIndex][1]);
        }

        xVisualSampleIndex += gain;
    }

    const QMatrix4x4 matrix = calculateMatrix(m_waveformRenderer, true);

    const int matrixLocation = m_shader.uniformLocation("matrix");
    const int colorLocation = m_shader.uniformLocation("color");
    const int positionLocation = m_shader.attributeLocation("position");

    m_shader.bind();
    m_shader.enableAttributeArray(positionLocation);

    m_shader.setUniformValue(matrixLocation, matrix);

    QColor colors[4];
    colors[0].setRgbF(static_cast<float>(m_rgbLowColor_r),
            static_cast<float>(m_rgbLowColor_g),
            static_cast<float>(m_rgbLowColor_b));
    colors[1].setRgbF(static_cast<float>(m_rgbMidColor_r),
            static_cast<float>(m_rgbMidColor_g),
            static_cast<float>(m_rgbMidColor_b));
    colors[2].setRgbF(static_cast<float>(m_rgbHighColor_r),
            static_cast<float>(m_rgbHighColor_g),
            static_cast<float>(m_rgbHighColor_b));
    colors[3].setRgbF(1.f, 1.f, 1.f);

    // 3 bands + 1 extra for the horizontal line

    for (int i = 0; i < 4; i++) {
        DEBUG_ASSERT(reserved[i] == m_verticesForGroup[i].size());
        m_shader.setUniformValue(colorLocation, colors[i]);
        m_shader.setAttributeArray(
                positionLocation, GL_FLOAT, m_verticesForGroup[i].constData(), 2);

        glDrawArrays(GL_TRIANGLES, 0, m_verticesForGroup[i].size());
    }

    m_shader.disableAttributeArray(positionLocation);
    m_shader.release();
}
