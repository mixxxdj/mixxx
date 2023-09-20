#include "waveform/renderers/allshader/waveformrenderersimple.h"

#include "track/track.h"
#include "util/math.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/waveform.h"
#include "waveform/waveformwidgetfactory.h"
#include "waveform/widgets/allshader/waveformwidget.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

using namespace allshader;

WaveformRendererSimple::WaveformRendererSimple(
        WaveformWidgetRenderer* waveformWidget)
        : WaveformRendererSignalBase(waveformWidget) {
}

void WaveformRendererSimple::onSetup(const QDomNode& node) {
    Q_UNUSED(node);
}

void WaveformRendererSimple::initializeGL() {
    WaveformRendererSignalBase::initializeGL();
    m_shader.init();
}

void WaveformRendererSimple::paintGL() {
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
    const double visualIncrementPerPixel =
            (lastVisualIndex - firstVisualIndex) / static_cast<double>(length);

    // Per-band gain from the EQ knobs.
    float allGain{1.0};
    float bandGain[3] = {1.0, 1.0, 1.0};
    getGains(&allGain, &bandGain[0], &bandGain[1], &bandGain[2]);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth()) * devicePixelRatio;
    const float halfBreadth = breadth / 2.0f;

    const float heightFactor = allGain * halfBreadth / 255.f;

    // Effective visual index of x
    double xVisualSampleIndex = firstVisualIndex;

    const int numVerticesPerLine = 6; // 2 triangles

    int reserved[2];

    reserved[0] = numVerticesPerLine * length;
    m_vertices[0].clear();
    m_vertices[0].reserve(reserved[0]);

    // the horizontal line
    reserved[1] = numVerticesPerLine;
    m_vertices[1].clear();
    m_vertices[1].reserve(reserved[1]);

    m_vertices[1].addRectangle(
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
        double maxSamplingRange = visualIncrementPerPixel / 2.0;

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

        // 2 channels
        float max[2]{};

        for (int i = visualIndexStart; i < visualIndexStop; i += 2) {
            for (int chn = 0; chn < 2; chn++) {
                const WaveformData& waveformData = data[i + chn];
                const float filteredAll = static_cast<float>(waveformData.filtered.all);

                max[chn] = math_max(max[chn], filteredAll);
            }
        }

        const float fpos = static_cast<float>(pos);

        // lines are thin rectangles
        m_vertices[0].addRectangle(
                fpos - 0.5f,
                halfBreadth - heightFactor * max[0],
                fpos + 0.5f,
                halfBreadth + heightFactor * max[1]);

        xVisualSampleIndex += visualIncrementPerPixel;
    }

    const QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, true);

    const int matrixLocation = m_shader.uniformLocation("matrix");
    const int colorLocation = m_shader.uniformLocation("color");
    const int positionLocation = m_shader.attributeLocation("position");

    m_shader.bind();
    m_shader.enableAttributeArray(positionLocation);

    m_shader.setUniformValue(matrixLocation, matrix);

    QColor colors[2];
    colors[0].setRgbF(static_cast<float>(m_signalColor_r),
            static_cast<float>(m_signalColor_g),
            static_cast<float>(m_signalColor_b));
    colors[1].setRgbF(static_cast<float>(m_axesColor_r),
            static_cast<float>(m_axesColor_g),
            static_cast<float>(m_axesColor_b),
            static_cast<float>(m_axesColor_a));

    for (int i = 0; i < 2; i++) {
        DEBUG_ASSERT(reserved[i] == m_vertices[i].size());
        m_shader.setUniformValue(colorLocation, colors[i]);
        m_shader.setAttributeArray(
                positionLocation, GL_FLOAT, m_vertices[i].constData(), 2);

        glDrawArrays(GL_TRIANGLES, 0, m_vertices[i].size());
    }

    m_shader.disableAttributeArray(positionLocation);
    m_shader.release();
}
