#include "waveform/renderers/allshader/waveformrenderersimple.h"

#include "track/track.h"
#include "util/math.h"
#include "waveform/renderers/allshader/matrixforwidgetgeometry.h"
#include "waveform/renderers/waveformwidgetrenderer.h"
#include "waveform/waveform.h"

namespace allshader {

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

    // Note that waveform refers to the visual waveform, not to audio samples.
    //
    // WaveformData* data contains the L and R waveform values interleaved. In the calculations
    // below, 'frame' refers to the index of such an L-R pair.
    const int visualFramesSize = dataSize / 2;

    // Calculate the first and last frame to draw, from the normalized display position
    const double firstVisualFrame =
            m_waveformRenderer->getFirstDisplayedPosition() * visualFramesSize;
    const double lastVisualFrame =
            m_waveformRenderer->getLastDisplayedPosition() * visualFramesSize;

    // Calculate the number of visual frames per horizontal pixel.
    const double visualIncrementPerPixel =
            (lastVisualFrame - firstVisualFrame) / static_cast<double>(length);

    // Per-band gain from the EQ knobs.
    float allGain{1.0};
    float bandGain[3] = {1.0, 1.0, 1.0};
    getGains(&allGain, false, &bandGain[0], &bandGain[1], &bandGain[2]);

    const float breadth = static_cast<float>(m_waveformRenderer->getBreadth()) * devicePixelRatio;
    const float halfBreadth = breadth / 2.0f;

    const float heightFactor = allGain * halfBreadth / m_maxValue;

    // Effective visual frame for x, which we will increment for each pixel advanced
    double xVisualFrame = qRound(firstVisualFrame / visualIncrementPerPixel) *
            visualIncrementPerPixel;

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

    // We will iterate over a range of waveform data, centered around xVisualFrame
    const double maxSamplingRange = visualIncrementPerPixel / 2.0;

    for (int pos = 0; pos < length; ++pos) {
        // Calculate the start and end of the range of waveform data, centered around xVisualFrame
        const int visualFrameStart = std::lround(xVisualFrame - maxSamplingRange);
        const int visualFrameStop = std::lround(xVisualFrame + maxSamplingRange);

        // Calculate the actual (deinterleaved) indices.
        //
        // Make sure we stay inside data at the lower boundary
        const int visualIndexStart = std::max(visualFrameStart * 2, 0);
        // and at the upper boundary.
        // Note: * dataSize - 1, because below we add chn = 1
        //       * visualFrameStart + 1, because we want to have at least 1 value
        const int visualIndexStop =
                std::min(std::max(visualFrameStop, visualFrameStart + 1) * 2, dataSize - 1);

        // 2 channels
        float max[2]{};

        for (int chn = 0; chn < 2; chn++) {
            // data is interleaved left / right
            for (int i = visualIndexStart + chn; i < visualIndexStop + chn; i += 2) {
                const WaveformData& waveformData = data[i];
                const float filteredAll = static_cast<float>(waveformData.filtered.all);
                // Uncomment to undo scaling with pow(value, 2.0f * 0.316f) done
                // in analyzerwaveform.h const float filteredAll =
                // unscale(waveformData.filtered.all);

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

        xVisualFrame += visualIncrementPerPixel;
    }

    const QMatrix4x4 matrix = matrixForWidgetGeometry(m_waveformRenderer, true);

    const int matrixLocation = m_shader.matrixLocation();
    const int colorLocation = m_shader.colorLocation();
    const int positionLocation = m_shader.positionLocation();

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

} // namespace allshader
